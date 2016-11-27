/***************************************************************************

                            -= Psikyo Games =-

                driver by   Luca Elia (l.elia@tin.it)


CPU:    68EC020 + PIC16C57 [Optional MCU]

Sound:  Z80A                +   YM2610
   Or:  LZ8420M (Z80 core)  +   YMF286-K (YM2610 compatible)

Chips:  PS2001B
        PS3103
        PS3204
        PS3305

---------------------------------------------------------------------------
Name                    Year    Board          Notes
---------------------------------------------------------------------------
Sengoku Ace         (J) 1993    SH201B
Gun Bird            (J) 1994    KA302C
Battle K-Road       (J) 1994    ""
Strikers 1945       (J) 1995    SH403/SH404    SH403 is similiar to KA302C
Tengai              (J) 1996    SH404          SH404 has MCU, ymf278-b for sound and gfx banking
---------------------------------------------------------------------------

To Do:

- Flip Screen support

***************************************************************************/

/***** Gun Bird Japan Crash Notes

The Following Section of Code in Gunbird causes reads from the
0x080000 - 0x0fffff region

002894: E2817000           asr.l   #1, D1
002896: 70001030           moveq   #$0, D0
002898: 10301804           move.b  ($4,A0,D1.l), D0   <-- *
00289C: 60183202           bra     28b6
00289E: 3202C2C7           move.w  D2, D1
0028A0: C2C77000           mulu.w  D7, D1
0028A2: 70003005           moveq   #$0, D0
0028A4: 3005D280           move.w  D5, D0
0028A6: D280E281           add.l   D0, D1
0028A8: E2812071           asr.l   #1, D1
0028AA: 20713515           movea.l ([A1],D3.w*4), A0
0028AE: 70001030           moveq   #$0, D0
0028B0: 10301804           move.b  ($4,A0,D1.l), D0   <-- *
0028B4: E880720F           asr.l   #4, D0
0028B6: 720FC041           moveq   #$f, D1

This causes Gunbird to crash if the ROM Region Size
allocated during loading is smaller than the ROM
region as it trys to read beyond the allocated rom region

This was pointed out by Bart Puype

*****/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2610intf.h"
#include "sound/ymf278b.h"
#include "sound/okim6295.h"
#include "includes/psikyo.h"


/***************************************************************************


                                Main CPU


***************************************************************************/

static CUSTOM_INPUT( z80_nmi_r )
{
	psikyo_state *state = (psikyo_state *)field->port->machine->driver_data;
	int ret = 0x00;

	if (state->z80_nmi)
	{
		ret = 0x01;

		/* main CPU might be waiting for sound CPU to finish NMI,
           so set a timer to give sound CPU a chance to run */
		timer_call_after_resynch(field->port->machine, NULL, 0, NULL);
//      logerror("%s - Read coin port during Z80 NMI\n", cpuexec_describe_context(machine));
	}

	return ret;
}

static CUSTOM_INPUT( mcu_status_r )
{
	psikyo_state *state = (psikyo_state *)field->port->machine->driver_data;
	int ret = 0x00;

	/* Don't know exactly what this bit is, but s1945 and tengai
        both spin waiting for it to go low during POST.  Also,
        the following code in tengai (don't know where or if it is
        reached) waits for it to pulse:

        01A546:  move.b  (A2), D0    ; A2 = $c00003
        01A548:  andi.b  #$4, D0
        01A54C:  beq     $1a546
        01A54E:  move.b  (A2), D0
        01A550:  andi.b  #$4, D0
        01A554:  bne     $1a54e

        Interestingly, s1945jn has the code that spins on this bit,
        but said code is never reached.  Prototype? */
	if (state->mcu_status)
		ret = 0x01;

	state->mcu_status = !state->mcu_status;	/* hack */

	return ret;
}

static READ32_HANDLER( sngkace_input_r )
{
	switch (offset)
	{
		case 0x0:	return input_port_read(space->machine, "P1_P2");
		case 0x1:	return input_port_read(space->machine, "DSW");
		case 0x2:	return input_port_read(space->machine, "COIN");
		default:	logerror("PC %06X - Read input %02X !\n", cpu_get_pc(space->cpu), offset * 2);
				return 0;
	}
}

static READ32_HANDLER( gunbird_input_r )
{
	switch (offset)
	{
		case 0x0:	return input_port_read(space->machine, "P1_P2");
		case 0x1:	return input_port_read(space->machine, "DSW");
		default:	logerror("PC %06X - Read input %02X !\n", cpu_get_pc(space->cpu), offset * 2);
				return 0;
	}
}


static TIMER_CALLBACK( psikyo_soundlatch_callback )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;
	state->soundlatch = param;
	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, ASSERT_LINE);
	state->z80_nmi = 1;
}

static WRITE32_HANDLER( psikyo_soundlatch_w )
{
	if (ACCESSING_BITS_0_7)
		timer_call_after_resynch(space->machine, NULL, data & 0xff, psikyo_soundlatch_callback);
}

/***************************************************************************
                        Strikers 1945 / Tengai
***************************************************************************/

static WRITE32_HANDLER( s1945_soundlatch_w )
{
	if (ACCESSING_BITS_16_23)
		timer_call_after_resynch(space->machine, NULL, (data >> 16) & 0xff, psikyo_soundlatch_callback);
}

static const UINT8 s1945_table[256] = {
	0x00, 0x00, 0x64, 0xae, 0x00, 0x00, 0x26, 0x2c, 0x00, 0x00, 0x2c, 0xda, 0x00, 0x00, 0x2c, 0xbc,
	0x00, 0x00, 0x2c, 0x9e, 0x00, 0x00, 0x2f, 0x0e, 0x00, 0x00, 0x31, 0x10, 0x00, 0x00, 0xc5, 0x1e,
	0x00, 0x00, 0x32, 0x90, 0x00, 0x00, 0xac, 0x5c, 0x00, 0x00, 0x2b, 0xc0
};

static const UINT8 s1945a_table[256] = {
	0x00, 0x00, 0x64, 0xbe, 0x00, 0x00, 0x26, 0x2c, 0x00, 0x00, 0x2c, 0xda, 0x00, 0x00, 0x2c, 0xbc,
	0x00, 0x00, 0x2c, 0x9e, 0x00, 0x00, 0x2f, 0x0e, 0x00, 0x00, 0x31, 0x10, 0x00, 0x00, 0xc7, 0x2a,
	0x00, 0x00, 0x32, 0x90, 0x00, 0x00, 0xad, 0x4c, 0x00, 0x00, 0x2b, 0xc0
};

static const UINT8 s1945j_table[256] = {
	0x00, 0x00, 0x64, 0xb6, 0x00, 0x00, 0x26, 0x2c, 0x00, 0x00, 0x2c, 0xda, 0x00, 0x00, 0x2c, 0xbc,
	0x00, 0x00, 0x2c, 0x9e, 0x00, 0x00, 0x2f, 0x0e, 0x00, 0x00, 0x31, 0x10, 0x00, 0x00, 0xc5, 0x92,
	0x00, 0x00, 0x32, 0x90, 0x00, 0x00, 0xac, 0x64, 0x00, 0x00, 0x2b, 0xc0
};

static WRITE32_HANDLER( s1945_mcu_w )
{
	psikyo_state *state = (psikyo_state *)space->machine->driver_data;

	// Accesses are always bytes, so resolve it
	int suboff;

	for (suboff = 0; suboff < 3; suboff++)
		if ((0xff << (8 * suboff)) & mem_mask)
			break;
	data >>= 8 * suboff;
	offset = offset * 4 + 4 + (3 - suboff);

	switch (offset)
	{
	case 0x06:
		state->s1945_mcu_inlatch = data;
		break;
	case 0x08:
		state->s1945_mcu_control = data;
		break;
	case 0x09:
		state->s1945_mcu_direction = data;
		break;
	case 0x07:
		psikyo_switch_banks(space->machine, 1, (data >> 6) & 3);
		psikyo_switch_banks(space->machine, 0, (data >> 4) & 3);
		state->s1945_mcu_bctrl = data;
		break;
	case 0x0b:
		switch (data | (state->s1945_mcu_direction ? 0x100 : 0))
		{
		case 0x11c:
			state->s1945_mcu_latching = 5;
			state->s1945_mcu_index = state->s1945_mcu_inlatch;
			break;
		case 0x013:
//          logerror("MCU: Table read index %02x\n", state->s1945_mcu_index);
			state->s1945_mcu_latching = 1;
			state->s1945_mcu_latch1 = state->s1945_mcu_table[state->s1945_mcu_index];
			break;
		case 0x113:
			state->s1945_mcu_mode = state->s1945_mcu_inlatch;
			if (state->s1945_mcu_mode == 1)
			{
				state->s1945_mcu_latching &= ~1;
				state->s1945_mcu_latch2 = 0x55;
			}
			else
			{
				// Go figure.
				state->s1945_mcu_latching &= ~1;
				state->s1945_mcu_latching |= 2;
			}
			state->s1945_mcu_latching &= ~4;
			state->s1945_mcu_latch1 = state->s1945_mcu_inlatch;
			break;
		case 0x010:
		case 0x110:
			state->s1945_mcu_latching |= 4;
			break;
		default:
//          logerror("MCU: function %02x, direction %02x, latch1 %02x, latch2 %02x (%x)\n", data, state->s1945_mcu_direction, state->s1945_mcu_latch1, state->s1945_mcu_latch2, cpu_get_pc(space->cpu));
			break;
		}
		break;
	default:
//      logerror("MCU.w %x, %02x (%x)\n", offset, data, cpu_get_pc(space->cpu));
		;
	}
}

static READ32_HANDLER( s1945_mcu_r )
{
	psikyo_state *state = (psikyo_state *)space->machine->driver_data;

	switch (offset)
	{
	case 0:
		{
		UINT32 res;
		if (state->s1945_mcu_control & 16)
		{
			res = state->s1945_mcu_latching & 4 ? 0x0000ff00 : state->s1945_mcu_latch1 << 8;
			state->s1945_mcu_latching |= 4;
		}
		else
		{
			res = state->s1945_mcu_latching & 1 ? 0x0000ff00 : state->s1945_mcu_latch2 << 8;
			state->s1945_mcu_latching |= 1;
		}
		res |= state->s1945_mcu_bctrl & 0xf0;
		return res;
	}
	case 1:
		return (state->s1945_mcu_latching << 24) | 0x08000000;
	}
	return 0;
}

static READ32_HANDLER( s1945_input_r )
{
	switch (offset)
	{
		case 0x0:	return input_port_read(space->machine, "P1_P2");
		case 0x1:	return (input_port_read(space->machine, "DSW") & 0xffff000f) | s1945_mcu_r(space, offset - 1, mem_mask);
		case 0x2:	return s1945_mcu_r(space, offset - 1, mem_mask);
		default:	logerror("PC %06X - Read input %02X !\n", cpu_get_pc(space->cpu), offset * 2);
					return 0;
	}
}


/***************************************************************************


                                Memory Map


***************************************************************************/

static WRITE32_HANDLER( paletteram32_xRRRRRGGGGGBBBBB_dword_w )
{
	if (ACCESSING_BITS_16_31)
		paletteram16_xRRRRRGGGGGBBBBB_word_w(space, offset * 2, data >> 16, mem_mask >> 16);
	if (ACCESSING_BITS_0_15)
		paletteram16_xRRRRRGGGGGBBBBB_word_w(space, offset * 2 + 1, data, mem_mask);
}

static ADDRESS_MAP_START( psikyo_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM														// ROM (not all used)
	AM_RANGE(0x400000, 0x401fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)		// Sprites, buffered by two frames (list buffered + fb buffered)
	AM_RANGE(0x600000, 0x601fff) AM_RAM_WRITE(paletteram32_xRRRRRGGGGGBBBBB_dword_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE(0x800000, 0x801fff) AM_RAM_WRITE(psikyo_vram_0_w) AM_BASE_MEMBER(psikyo_state, vram_0)		// Layer 0
	AM_RANGE(0x802000, 0x803fff) AM_RAM_WRITE(psikyo_vram_1_w) AM_BASE_MEMBER(psikyo_state, vram_1)		// Layer 1
	AM_RANGE(0x804000, 0x807fff) AM_RAM AM_BASE_MEMBER(psikyo_state, vregs)							// RAM + Vregs
//  AM_RANGE(0xc00000, 0xc0000b) AM_READ(psikyo_input_r)                                    // Depends on board, see DRIVER_INIT
//  AM_RANGE(0xc00004, 0xc0000b) AM_WRITE(s1945_mcu_w)                                      // MCU on sh404, see DRIVER_INIT
//  AM_RANGE(0xc00010, 0xc00013) AM_WRITE(psikyo_soundlatch_w)                              // Depends on board, see DRIVER_INIT
	AM_RANGE(0xfe0000, 0xffffff) AM_RAM														// RAM
ADDRESS_MAP_END

static READ32_DEVICE_HANDLER( s1945bl_oki_r )
{
	UINT8 dat = okim6295_r(device, 0);
	return dat << 24;
}

static WRITE32_DEVICE_HANDLER( s1945bl_oki_w )
{
	if (ACCESSING_BITS_24_31)
		okim6295_w(device, 0, data >> 24);

	if (ACCESSING_BITS_16_23)
	{
		// not at all sure about this, it seems to write 0 too often
		UINT8 bank = (data & 0x00ff0000) >> 16;
		if (bank < 4)
			memory_set_bank(device->machine, "okibank", bank);
	}

	if (ACCESSING_BITS_8_15)
		printf("ACCESSING_BITS_8_15 ?? %08x %08x\n", data & 0x00ff0000, mem_mask);

	if (ACCESSING_BITS_0_7)
		printf("ACCESSING_BITS_0_7 ?? %08x %08x\n", data & 0x00ff0000, mem_mask);
}

static ADDRESS_MAP_START( s1945bl_oki_map, 0, 8 )
	AM_RANGE(0x00000, 0x2ffff) AM_ROM
	AM_RANGE(0x30000, 0x3ffff) AM_ROMBANK("okibank")
ADDRESS_MAP_END

static ADDRESS_MAP_START( psikyo_bootleg_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM														// ROM (not all used)
	AM_RANGE(0x200000, 0x200fff) AM_RAM AM_BASE_MEMBER(psikyo_state, bootleg_spritebuffer)				// RAM (it copies the spritelist here, the HW probably doesn't have automatic buffering like the originals?

	AM_RANGE(0x400000, 0x401fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)		// Sprites, buffered by two frames (list buffered + fb buffered)
	AM_RANGE(0x600000, 0x601fff) AM_RAM_WRITE(paletteram32_xRRRRRGGGGGBBBBB_dword_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE(0x800000, 0x801fff) AM_RAM_WRITE(psikyo_vram_0_w) AM_BASE_MEMBER(psikyo_state, vram_0)		// Layer 0
	AM_RANGE(0x802000, 0x803fff) AM_RAM_WRITE(psikyo_vram_1_w) AM_BASE_MEMBER(psikyo_state, vram_1)		// Layer 1
	AM_RANGE(0x804000, 0x807fff) AM_RAM AM_BASE_MEMBER(psikyo_state, vregs)								// RAM + Vregs
//  AM_RANGE(0xc00000, 0xc0000b) AM_READ(psikyo_input_r)                                    // Depends on board, see DRIVER_INIT
//  AM_RANGE(0xc00004, 0xc0000b) AM_WRITE(s1945_mcu_w)                                      // MCU on sh404, see DRIVER_INIT
//  AM_RANGE(0xc00010, 0xc00013) AM_WRITE(psikyo_soundlatch_w)                              // Depends on board, see DRIVER_INIT

	AM_RANGE(0xC00018, 0xC0001b) AM_DEVREADWRITE("oki", s1945bl_oki_r, s1945bl_oki_w)

	AM_RANGE(0xfe0000, 0xffffff) AM_RAM														// RAM

ADDRESS_MAP_END

/***************************************************************************


                                Sound CPU


***************************************************************************/

static void sound_irq( running_device *device, int irq )
{
	psikyo_state *state = (psikyo_state *)device->machine->driver_data;
	cpu_set_input_line(state->audiocpu, 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static READ8_HANDLER( psikyo_soundlatch_r )
{
	psikyo_state *state = (psikyo_state *)space->machine->driver_data;
	return state->soundlatch;
}

static WRITE8_HANDLER( psikyo_clear_nmi_w )
{
	psikyo_state *state = (psikyo_state *)space->machine->driver_data;
	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, CLEAR_LINE);
	state->z80_nmi = 0;
}


/***************************************************************************
                        Sengoku Ace / Samurai Aces
***************************************************************************/

static WRITE8_HANDLER( sngkace_sound_bankswitch_w )
{
	memory_set_bank(space->machine, "bank1", data & 0x03);
}

static ADDRESS_MAP_START( sngkace_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x77ff) AM_ROM							// ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM							// RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")					// Banked ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sngkace_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ymsnd", ym2610_r, ym2610_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(sngkace_sound_bankswitch_w)
	AM_RANGE(0x08, 0x08) AM_READ(psikyo_soundlatch_r)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(psikyo_clear_nmi_w)
ADDRESS_MAP_END


/***************************************************************************
                                Gun Bird
***************************************************************************/

static WRITE8_HANDLER( gunbird_sound_bankswitch_w )
{
	memory_set_bank(space->machine, "bank1", (data >> 4) & 0x03);
}

static ADDRESS_MAP_START( gunbird_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM							// ROM
	AM_RANGE(0x8000, 0x81ff) AM_RAM							// RAM
	AM_RANGE(0x8200, 0xffff) AM_ROMBANK("bank1")					// Banked ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( gunbird_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(gunbird_sound_bankswitch_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ymsnd", ym2610_r, ym2610_w)
	AM_RANGE(0x08, 0x08) AM_READ(psikyo_soundlatch_r)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(psikyo_clear_nmi_w)
ADDRESS_MAP_END

/***************************************************************************
                        Strikers 1945 / Tengai
***************************************************************************/

static ADDRESS_MAP_START( s1945_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(gunbird_sound_bankswitch_w)
	AM_RANGE(0x02, 0x03) AM_WRITENOP
	AM_RANGE(0x08, 0x0d) AM_DEVREADWRITE("ymf", ymf278b_r, ymf278b_w)
	AM_RANGE(0x10, 0x10) AM_READ(psikyo_soundlatch_r)
	AM_RANGE(0x18, 0x18) AM_WRITE(psikyo_clear_nmi_w)
ADDRESS_MAP_END

/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( psikyo_common )
	PORT_START("P1_P2")		/* c00000&1 */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )	// these depends by the games
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("DSW")		/* c00004 -> c00007 */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )	// these depends by the games
	PORT_DIPNAME( 0x00010000, 0x00010000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(          0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00000000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(          0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c0000, 0x000c0000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(          0x00080000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x000c0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00040000, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x00300000, 0x00300000, DEF_STR( Lives ) )
	PORT_DIPSETTING(          0x00200000, "1" )
	PORT_DIPSETTING(          0x00100000, "2" )
	PORT_DIPSETTING(          0x00300000, "3" )
	PORT_DIPSETTING(          0x00000000, "4" )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(          0x00400000, "400K" )
	PORT_DIPSETTING(          0x00000000, "600K" )
	PORT_SERVICE( 0x00800000, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x01000000, 0x01000000, "Credits/Coinage" )	// [Free Play] on all for free play
	PORT_DIPSETTING(          0x01000000, "A+B/A&B" )
	PORT_DIPSETTING(          0x00000000, "A&B/A [Free Play]" )
	PORT_DIPNAME( 0x0e000000, 0x0e000000, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(          0x0a000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x0c000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x0e000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x08000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x06000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x04000000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(          0x02000000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(          0x00000000, "1C 6C [Free Play]" )
	PORT_DIPNAME( 0x70000000, 0x70000000, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(          0x50000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x60000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x70000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x40000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x30000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x20000000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(          0x10000000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(          0x00000000, "1C 6C [Free Play]" )
	PORT_DIPNAME( 0x80000000, 0x80000000, "2C Start, 1C Continue" )
	PORT_DIPSETTING(          0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, "On [Free Play]" ) // Forces 1C_1C
INPUT_PORTS_END



/***************************************************************************
                        Samurai Aces / Sengoku Ace (Japan)
***************************************************************************/

static INPUT_PORTS_START( samuraia )
	PORT_INCLUDE(psikyo_common)

	PORT_START("COIN")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_VBLANK  )	// vblank
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )	// unused?
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )	// unused?
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(z80_nmi_r, NULL)	// From Sound CPU
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )	// unused?

	PORT_MODIFY("DSW")		/* c00004 -> c00007 */
	/***********************************************

    This Dip port is bit based:

    Bit 0 1 2 3
        1 1 1 1 World

        0 1 1 1 USA With FBI logo
        1 0 1 1 Korea With FBI logo??
        1 1 0 1 Hong Kong With FBI logo??
        1 1 1 0 Taiwan With FBI logo??

    ************************************************/
	PORT_DIPNAME( 0x000000ff, 0x000000ff, "Country" )
	PORT_DIPSETTING(          0x000000ff, DEF_STR( World ) )
	PORT_DIPSETTING(          0x000000ef, "USA & Canada" )
	PORT_DIPSETTING(          0x000000df, "Korea" )
	PORT_DIPSETTING(          0x000000bf, "Hong Kong" )
	PORT_DIPSETTING(          0x0000007f, "Taiwan" )
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )	// unused?
INPUT_PORTS_END

static INPUT_PORTS_START( sngkace )
	PORT_INCLUDE( samuraia )

	PORT_MODIFY("DSW")		/* c00004 -> c00007 */
	/***********************************************

    This Dip port is bit based:

    Bit 0 1 2 3
        1 1 1 1 Japan

        0 1 1 1 USA With FBI logo
        1 0 1 1 Korea
        1 1 0 1 Hong Kong
        1 1 1 0 Taiwan

    ************************************************/
#if 0 // See Patch in MACHINE_RESET, only text not logo
	PORT_DIPNAME( 0x000000ff, 0x000000ff, "Country" )
	PORT_DIPSETTING(          0x000000ff, DEF_STR( Japan ) )
	PORT_DIPSETTING(          0x000000ef, "USA & Canada" )
	PORT_DIPSETTING(          0x000000df, "Korea" )
	PORT_DIPSETTING(          0x000000bf, "Hong Kong" )
	PORT_DIPSETTING(          0x0000007f, "Taiwan" )
#endif
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Battle K-Road
***************************************************************************/

static INPUT_PORTS_START( btlkroad )
	PORT_INCLUDE(psikyo_common)

	PORT_MODIFY("P1_P2")			/* c00000 -> c00003 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(z80_nmi_r, NULL)	// From Sound CPU
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_MODIFY("DSW")		/* c00004 -> c00007 */
	/***********************************************

    Bit 0 1 2 3
        1 1 1 1 Japan

        0 1 1 1 USA & Canada
        0 0 1 1 Korea
        0 1 0 1 Hong Kong
        0 1 1 0 Taiwan
        Other   World

    ************************************************/
	PORT_DIPNAME( 0x0000000f, 0x00000000, "Copyright (Country)" )
	PORT_DIPSETTING(          0x0000000f, "Psikyo (Japan)" )
	PORT_DIPSETTING(          0x0000000e, "Jaleco+Psikyo (USA & Canada)" )
	PORT_DIPSETTING(          0x0000000c, "Psikyo (Korea)" )
	PORT_DIPSETTING(          0x0000000a, "Psikyo (Hong Kong)" )
	PORT_DIPSETTING(          0x00000006, "Psikyo (Taiwan)" )
	PORT_DIPSETTING(          0x00000000, "Psikyo (World)" )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_VBLANK  )	// vblank   ACTIVE_HIGH fixes slowdowns, but is it right?
	// This DSW is used for debugging the game
	PORT_DIPNAME( 0x00000100, 0x00000100, "Unknown 3-0" )	// tested!
	PORT_DIPSETTING(          0x00000100, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000200, 0x00000200, "Unknown 3-1" )	// tested!
	PORT_DIPSETTING(          0x00000200, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000400, 0x00000400, "Unknown 3-2" )
	PORT_DIPSETTING(          0x00000400, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000800, 0x00000800, "Unknown 3-3" )
	PORT_DIPSETTING(          0x00000800, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00001000, 0x00001000, "Unknown 3-4" )	// tested!
	PORT_DIPSETTING(          0x00001000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00002000, 0x00002000, "Unknown 3-5" )
	PORT_DIPSETTING(          0x00002000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00004000, 0x00004000, "Unknown 3-6" )
	PORT_DIPSETTING(          0x00004000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00008000, 0x00008000, "Unknown 3-7" )	// tested!
	PORT_DIPSETTING(          0x00008000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )

	PORT_DIPNAME( 0x00100000, 0x00100000, "Unknown 2-4" )	// used
	PORT_DIPSETTING(          0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "Unknown 2-5" )	// used (energy lost?)
	PORT_DIPSETTING(          0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "Use DSW 3 (Debug)" )
	PORT_DIPSETTING(          0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************
                                Gun Bird
***************************************************************************/

static INPUT_PORTS_START( gunbird )
	PORT_INCLUDE(psikyo_common)

	PORT_MODIFY("P1_P2")			/* c00000 -> c00003 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(z80_nmi_r, NULL)	// From Sound CPU
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW")		/* c00004 -> c00007 */
	/***********************************************

    This Dip port is bit based:

    Bit 0 1 2 3
        1 1 1 1 World (No "For use in ...." screen)

        0 x x x USA With FBI logo
        1 0 x x Korea
        1 1 0 x Hong Kong
        1 1 1 0 Taiwan

        x = Doesn't Matter, see routine starting at 0108A4:

    Japan is listed in the code but how do you activate it?

    Has no effects on Japan or Korea versions.

    ************************************************/
	PORT_DIPNAME( 0x0000000f, 0x0000000f, "Country" )
	PORT_DIPSETTING(          0x0000000f, DEF_STR( World ) )
	PORT_DIPSETTING(          0x0000000e, DEF_STR( USA ) )
	PORT_DIPSETTING(          0x0000000d, "Korea" )
	PORT_DIPSETTING(          0x0000000b, "Hong Kong" )
	PORT_DIPSETTING(          0x00000007, "Taiwan" )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_VBLANK  )	// vblank
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )	// tested!
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )	// tested!
INPUT_PORTS_END

static INPUT_PORTS_START( gunbirdj )
	PORT_INCLUDE( gunbird )

	PORT_MODIFY("DSW")		/* c00004 -> c00007 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Strikers 1945
***************************************************************************/

static INPUT_PORTS_START( s1945 )
	PORT_INCLUDE(psikyo_common)

	PORT_MODIFY("P1_P2")			/* c00000 -> c00003 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(mcu_status_r, NULL)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(z80_nmi_r, NULL)	// From Sound CPU
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW")		/* c00004 -> c00007 */
	/***********************************************

    This Dip port is bit based:

    Bit 0 1 2 3
        1 1 1 1 World (No "For use in ...." screen), see:
                        0149B8: tst.w   $fffe58a0.l

        0 1 1 1 USA & Canada
        1 0 1 1 Korea
        1 1 0 1 Hong Kong
        1 1 1 0 Taiwan
                    Other regions check see:
                        005594: move.w  $fffe58a0.l, D0

    Came from a Japan board apparently!!!
    Japan is listed in the code but how do you activate it?
    No effect on set s1945j or s1945k

    ************************************************/
	PORT_DIPNAME( 0x0000000f, 0x0000000f, "Country" )
	PORT_DIPSETTING(          0x0000000f, DEF_STR( World ) )
	PORT_DIPSETTING(          0x0000000e, "U.S.A & Canada" )
	PORT_DIPSETTING(          0x0000000d, "Korea" )
	PORT_DIPSETTING(          0x0000000b, "Hong Kong" )
	PORT_DIPSETTING(          0x00000007, "Taiwan" )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_VBLANK  )	// vblank
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )	// tested!
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )	// tested!

	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(          0x00400000, "600K" )
	PORT_DIPSETTING(          0x00000000, "800K" )
	PORT_DIPNAME( 0x0e000000, 0x0e000000, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(          0x0a000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x0c000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x0e000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x08000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x06000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x04000000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(          0x02000000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x70000000, 0x70000000, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(          0x50000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x60000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x70000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x40000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x30000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x20000000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(          0x10000000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80000000, 0x80000000, "2C Start, 1C Continue" )
	PORT_DIPSETTING(          0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) ) // Forces 1C_1C
INPUT_PORTS_END

static INPUT_PORTS_START( s1945a )
	PORT_INCLUDE( s1945 )

	PORT_MODIFY("DSW")
	/***********************************************

    This Dip port is bit based:

    Bit 0 1 2 3
        1 1 1 1 Japan, anything but 0x0f = "World"
    ************************************************/
	PORT_DIPNAME( 0x0000000f, 0x0000000f, "Country" )
	PORT_DIPSETTING(          0x0000000f, DEF_STR( Japan ) )
	PORT_DIPSETTING(          0x0000000e, DEF_STR( World ) )
INPUT_PORTS_END

static INPUT_PORTS_START( s1945j )
	PORT_INCLUDE( s1945 )

	PORT_MODIFY("DSW")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( s1945bl )
	PORT_INCLUDE( s1945 )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	// I need to invert the Vblank on this to avoid excessive slowdown
	PORT_MODIFY("DSW")		/* c00004 -> c00007 */
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_VBLANK  )	// vblank
INPUT_PORTS_END


/***************************************************************************
                                Tengai
***************************************************************************/

static INPUT_PORTS_START( tengai )
	PORT_INCLUDE(psikyo_common)

	PORT_MODIFY("P1_P2")			/* c00000 -> c00003 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(mcu_status_r, NULL)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(z80_nmi_r, NULL)	// From Sound CPU
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW")		/* c00004 -> c00007 */
	/***********************************************

    This Dip port is bit based:

    Bit 0 1 2 3
        1 1 1 1 World
        0 1 1 1 USA & Canada
        1 0 1 1 Korea
        1 1 0 1 Hong Kong
        1 1 1 0 Taiwan

    ************************************************/
	PORT_DIPNAME( 0x0000000f, 0x0000000f, "Country" )
	PORT_DIPSETTING(          0x0000000f, DEF_STR( World ) )
	PORT_DIPSETTING(          0x0000000e, "U.S.A & Canada" )
	PORT_DIPSETTING(          0x0000000d, "Korea" )
	PORT_DIPSETTING(          0x0000000b, "Hong Kong" )
	PORT_DIPSETTING(          0x00000007, "Taiwan" )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_VBLANK  )	// vblank
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )	// tested!
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )	// tested!

	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(          0x00400000, "600K" )
	PORT_DIPSETTING(          0x00000000, "800K" )
INPUT_PORTS_END

static INPUT_PORTS_START( tengaij )
	PORT_INCLUDE( tengai )

	PORT_MODIFY("DSW")
	/***********************************************

    This Dip port is bit based:

    Bit 0 1 2 3
        1 1 1 1 Japan, anything but 0x0f = "World"

    Text for other regions is present though.

    ************************************************/
	PORT_DIPNAME( 0x0000000f, 0x0000000f, "Country" )
	PORT_DIPSETTING(          0x0000000f, DEF_STR( Japan ) )
	PORT_DIPSETTING(          0x0000000e, DEF_STR( World ) )
INPUT_PORTS_END


/***************************************************************************


                                Gfx Layouts


***************************************************************************/

static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{0,1,2,3},
	{2*4,3*4,0*4,1*4,6*4,7*4,4*4,5*4,
	 10*4,11*4,8*4,9*4,14*4,15*4,12*4,13*4},
	{0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64,
	 8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64},
	16*16*4
};

static GFXDECODE_START( psikyo )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4, 0x000, 0x20 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4, 0x800, 0x48 ) // [1] Layer 0 + 1
GFXDECODE_END



/***************************************************************************


                                Machine Drivers


***************************************************************************/

static MACHINE_START( psikyo )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	state->audiocpu = machine->device("audiocpu");

	state_save_register_global(machine, state->soundlatch);
	state_save_register_global(machine, state->z80_nmi);
	state_save_register_global(machine, state->mcu_status);
	state_save_register_global(machine, state->tilemap_0_bank);
	state_save_register_global(machine, state->tilemap_1_bank);
}

static MACHINE_RESET( psikyo )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	state->soundlatch = 0;
	state->z80_nmi = 0;
	state->mcu_status = 0;
}


/***************************************************************************
                            Samurai Ace / Sengoku Aces
***************************************************************************/


static const ym2610_interface sngkace_ym2610_interface =
{
	sound_irq
};

static MACHINE_DRIVER_START( sngkace )

	/* driver data */
	MDRV_DRIVER_DATA(psikyo_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68EC020, XTAL_32MHz/2) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(psikyo_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_32MHz/8) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(sngkace_sound_map)
	MDRV_CPU_IO_MAP(sngkace_sound_io_map)

	MDRV_MACHINE_START(psikyo)
	MDRV_MACHINE_RESET(psikyo)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(59.3)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)	// we're using IPT_VBLANK
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-32-1)

	MDRV_GFXDECODE(psikyo)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(sngkace)
	MDRV_VIDEO_EOF(psikyo)
	MDRV_VIDEO_UPDATE(psikyo)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610, XTAL_32MHz/4) /* verified on pcb */
	MDRV_SOUND_CONFIG(sngkace_ym2610_interface)
	MDRV_SOUND_ROUTE(0, "lspeaker",  1.2)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.2)
	MDRV_SOUND_ROUTE(1, "lspeaker",  1.0)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_DRIVER_END



/***************************************************************************
        Gun Bird / Battle K-Road / Strikers 1945 (Japan, unprotected)
***************************************************************************/


static const ym2610_interface gunbird_ym2610_interface =
{
	sound_irq	/* irq */
};

static MACHINE_DRIVER_START( gunbird )

	/* driver data */
	MDRV_DRIVER_DATA(psikyo_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68EC020, 16000000)
	MDRV_CPU_PROGRAM_MAP(psikyo_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 4000000)	/* ! LZ8420M (Z80 core) ! */
	MDRV_CPU_PROGRAM_MAP(gunbird_sound_map)
	MDRV_CPU_IO_MAP(gunbird_sound_io_map)

	MDRV_MACHINE_START(psikyo)
	MDRV_MACHINE_RESET(psikyo)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(59.3)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)	// we're using IPT_VBLANK
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-32-1)

	MDRV_GFXDECODE(psikyo)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(psikyo)
	MDRV_VIDEO_EOF(psikyo)
	MDRV_VIDEO_UPDATE(psikyo)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610, 8000000)
	MDRV_SOUND_CONFIG(gunbird_ym2610_interface)
	MDRV_SOUND_ROUTE(0, "lspeaker",  1.2)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.2)
	MDRV_SOUND_ROUTE(1, "lspeaker",  1.0)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( s1945bl ) /* Bootleg hardware based on the unprotected Japanese Strikers 1945 set */

	/* driver data */
	MDRV_DRIVER_DATA(psikyo_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68EC020, 16000000)
	MDRV_CPU_PROGRAM_MAP(psikyo_bootleg_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_MACHINE_START(psikyo)
	MDRV_MACHINE_RESET(psikyo)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(59.3)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)	// we're using IPT_VBLANK
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-32-1)

	MDRV_GFXDECODE(psikyo)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(psikyo)
	MDRV_VIDEO_UPDATE(psikyo_bootleg)
	MDRV_VIDEO_EOF(psikyo)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", XTAL_16MHz/16, OKIM6295_PIN7_LOW) // ?? clock
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_DEVICE_ADDRESS_MAP(0, s1945bl_oki_map)
MACHINE_DRIVER_END



/***************************************************************************
                        Strikers 1945 / Tengai
***************************************************************************/


static void irqhandler( running_device *device, int linestate )
{
	psikyo_state *state = (psikyo_state *)device->machine->driver_data;
	cpu_set_input_line(state->audiocpu, 0, linestate ? ASSERT_LINE : CLEAR_LINE);
}

static const ymf278b_interface ymf278b_config =
{
	irqhandler
};

static MACHINE_DRIVER_START( s1945 )

	/* driver data */
	MDRV_DRIVER_DATA(psikyo_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68EC020, 16000000)
	MDRV_CPU_PROGRAM_MAP(psikyo_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 4000000)	/* ! LZ8420M (Z80 core) ! */
	MDRV_CPU_PROGRAM_MAP(gunbird_sound_map)
	MDRV_CPU_IO_MAP(s1945_sound_io_map)

	/* MCU should go here */

	MDRV_MACHINE_START(psikyo)
	MDRV_MACHINE_RESET(psikyo)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(59.90)    /* verified on pcb */
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)	// we're using IPT_VBLANK
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-32-1)

	MDRV_GFXDECODE(psikyo)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(psikyo)
	MDRV_VIDEO_EOF(psikyo)
	MDRV_VIDEO_UPDATE(psikyo)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymf", YMF278B, YMF278B_STD_CLOCK)
	MDRV_SOUND_CONFIG(ymf278b_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END





/***************************************************************************


                                ROMs Loading


***************************************************************************/


/***************************************************************************

                                Samurai Aces
                          ( WORLD/USA/HK/KOREA/TAIWAN Ver.)

                                Sengoku Ace
                          (Samurai Aces JPN Ver.)

Board:  SH201B
CPU:    TMP68EC020F-16
Sound:  Z80A + YM2610
OSC:    32.000, 14.31818 MHz

***************************************************************************/

ROM_START( samuraia )

	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "4-u127.bin", 0x000000, 0x040000, CRC(8c9911ca) SHA1(821ba648b9a1d495c600cbf4606f2dbddc6f9e6f) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "5-u126.bin", 0x000002, 0x040000, CRC(d20c3ef0) SHA1(264e5a7e45e130a9e7152468733337668dc5b65f) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "3-u58.bin", 0x00000, 0x20000, CRC(310f5c76) SHA1(dbfd1c5a7a514bccd89fc4f7191744cf76bb745d) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u14.bin",  0x000000, 0x200000, CRC(00a546cb) SHA1(30a8679b49928d5fcbe58b5eecc2ebd08173adf8) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layers 0 + 1 */
	ROM_LOAD( "u34.bin",  0x000000, 0x100000, CRC(e6a75bd8) SHA1(1aa84ea54584b6c8b2846194b48bf6d2afa67fee) )
	ROM_LOAD( "u35.bin",  0x100000, 0x100000, CRC(c4ca0164) SHA1(c75422de2e0127cdc23d8c223b674a5bd85b00fb) )

	ROM_REGION( 0x100000, "ymsnd", 0 )	/* Samples */
	ROM_LOAD( "u68.bin",  0x000000, 0x100000, CRC(9a7f6c34) SHA1(c549b209bce1d2c6eeb512db198ad20c3f5fb0ea) )

	ROM_REGION( 0x040000, "spritelut", 0 )	/* Sprites LUT */
	ROM_LOAD( "u11.bin",  0x000000, 0x040000, CRC(11a04d91) SHA1(5d146a9a39a70f2ee212ceab9a5469598432449e) ) // x1xxxxxxxxxxxxxxxx = 0xFF

ROM_END

ROM_START( sngkace )

	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "1-u127.bin", 0x000000, 0x040000, CRC(6c45b2f8) SHA1(08473297e174f3a6d67043f3b16f4e6b9c68b826) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "2-u126.bin", 0x000002, 0x040000, CRC(845a6760) SHA1(3b8fed294e28d9d8ef5cb5ec88b9ade396146a48) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "3-u58.bin", 0x00000, 0x20000, CRC(310f5c76) SHA1(dbfd1c5a7a514bccd89fc4f7191744cf76bb745d) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u14.bin",  0x000000, 0x200000, CRC(00a546cb) SHA1(30a8679b49928d5fcbe58b5eecc2ebd08173adf8) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layers 0 + 1 */
	ROM_LOAD( "u34.bin",  0x000000, 0x100000, CRC(e6a75bd8) SHA1(1aa84ea54584b6c8b2846194b48bf6d2afa67fee) )
	ROM_LOAD( "u35.bin",  0x100000, 0x100000, CRC(c4ca0164) SHA1(c75422de2e0127cdc23d8c223b674a5bd85b00fb) )

	ROM_REGION( 0x100000, "ymsnd", 0 )	/* Samples */
	ROM_LOAD( "u68.bin",  0x000000, 0x100000, CRC(9a7f6c34) SHA1(c549b209bce1d2c6eeb512db198ad20c3f5fb0ea) )

	ROM_REGION( 0x040000, "spritelut", 0 )	/* Sprites LUT */
	ROM_LOAD( "u11.bin",  0x000000, 0x040000, CRC(11a04d91) SHA1(5d146a9a39a70f2ee212ceab9a5469598432449e) ) // x1xxxxxxxxxxxxxxxx = 0xFF

ROM_END


/***************************************************************************

                                Gun Bird (Korea)
                                Gun Bird (Japan)
                            Battle K-Road (Japan)

Board:  KA302C
CPU:    MC68EC020FG16
Sound:  LZ8420M (Z80 core) + YMF286-K
OSC:    16.000, 14.31818 MHz

Chips:  PS2001B
        PS3103
        PS3204
        PS3305

***************************************************************************/

ROM_START( gunbird )

	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "4-u46.bin", 0x000000, 0x040000, CRC(b78ec99d) SHA1(399b79931652d9df1632cd4d7ec3d214e473a5c3) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "5-u39.bin", 0x000002, 0x040000, CRC(925f095d) SHA1(301a536119a0320a756e9c6e51fb10e36b90ef16) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "3-u71.bin", 0x00000, 0x20000, CRC(2168e4ba) SHA1(ca7ad6acb5f806ce2528e7b52c19e8cceecb8543) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x700000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u14.bin",  0x000000, 0x200000, CRC(7d7e8a00) SHA1(9f35f5b54ae928e9bf2aa6ad4604f669857955ec) )
	ROM_LOAD( "u24.bin",  0x200000, 0x200000, CRC(5e3ffc9d) SHA1(c284eb9ef56c8e6261fe11f91a10c5c5a56c9803) )
	ROM_LOAD( "u15.bin",  0x400000, 0x200000, CRC(a827bfb5) SHA1(6e02436e12085016cf1982c9ae07b6c6dec82f1b) )
	ROM_LOAD( "u25.bin",  0x600000, 0x100000, CRC(ef652e0c) SHA1(6dd994a15ced31d1bbd1a3b0e9d8d86eca33e217) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layers 0 + 1 */
	ROM_LOAD( "u33.bin",  0x000000, 0x200000, CRC(54494e6b) SHA1(f5d090d2d34d908b56b53a246def194929eba990) )

	ROM_REGION( 0x100000, "ymsnd", 0 )	/* ADPCM Samples */
	ROM_LOAD( "u56.bin",  0x000000, 0x100000, CRC(9e07104d) SHA1(3bc54cb755bb3194197706965b532d62b48c4d12) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )	/* DELTA-T Samples */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(e187ed4f) SHA1(05060723d89b1d05714447a14b5f5888ff3c2306) )

	ROM_REGION( 0x040000, "spritelut", 0 )	/* Sprites LUT */
	ROM_LOAD( "u3.bin",  0x000000, 0x040000, CRC(0905aeb2) SHA1(8cca09f7dfe3f804e77515f7b1b1bdbeb7bb3d80) )

ROM_END

ROM_START( gunbirdk )

	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "1k-u46.bin", 0x000000, 0x080000, CRC(745cee52) SHA1(6c5bb92c92c55f882484417bc1aa580684019610) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "2k-u39.bin", 0x000002, 0x080000, CRC(669632fb) SHA1(885dea42e6da35e9166a208b18dbd930642c26cd) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "k3-u71.bin", 0x00000, 0x20000, CRC(11994055) SHA1(619776c178361f23de37ff14e87284ec0f1f4f10) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x700000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u14.bin",  0x000000, 0x200000, CRC(7d7e8a00) SHA1(9f35f5b54ae928e9bf2aa6ad4604f669857955ec) )
	ROM_LOAD( "u24.bin",  0x200000, 0x200000, CRC(5e3ffc9d) SHA1(c284eb9ef56c8e6261fe11f91a10c5c5a56c9803) )
	ROM_LOAD( "u15.bin",  0x400000, 0x200000, CRC(a827bfb5) SHA1(6e02436e12085016cf1982c9ae07b6c6dec82f1b) )
	ROM_LOAD( "u25.bin",  0x600000, 0x100000, CRC(ef652e0c) SHA1(6dd994a15ced31d1bbd1a3b0e9d8d86eca33e217) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layers 0 + 1 */
	ROM_LOAD( "u33.bin",  0x000000, 0x200000, CRC(54494e6b) SHA1(f5d090d2d34d908b56b53a246def194929eba990) )

	ROM_REGION( 0x100000, "ymsnd", 0 )	/* ADPCM Samples */
	ROM_LOAD( "u56.bin",  0x000000, 0x100000, CRC(9e07104d) SHA1(3bc54cb755bb3194197706965b532d62b48c4d12) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )	/* DELTA-T Samples */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(e187ed4f) SHA1(05060723d89b1d05714447a14b5f5888ff3c2306) )

	ROM_REGION( 0x040000, "spritelut", 0 )	/* Sprites LUT */
	ROM_LOAD( "u3.bin",  0x000000, 0x040000, CRC(0905aeb2) SHA1(8cca09f7dfe3f804e77515f7b1b1bdbeb7bb3d80) )

ROM_END

ROM_START( gunbirdj )

	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "1-u46.bin", 0x000000, 0x040000, CRC(474abd69) SHA1(27f37333075f9c92849101aad4875e69004d747b) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "2-u39.bin", 0x000002, 0x040000, CRC(3e3e661f) SHA1(b5648546f390539b0f727a9a62d1b9516254ae21) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "3-u71.bin", 0x00000, 0x20000, CRC(2168e4ba) SHA1(ca7ad6acb5f806ce2528e7b52c19e8cceecb8543) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x700000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u14.bin",  0x000000, 0x200000, CRC(7d7e8a00) SHA1(9f35f5b54ae928e9bf2aa6ad4604f669857955ec) )
	ROM_LOAD( "u24.bin",  0x200000, 0x200000, CRC(5e3ffc9d) SHA1(c284eb9ef56c8e6261fe11f91a10c5c5a56c9803) )
	ROM_LOAD( "u15.bin",  0x400000, 0x200000, CRC(a827bfb5) SHA1(6e02436e12085016cf1982c9ae07b6c6dec82f1b) )
	ROM_LOAD( "u25.bin",  0x600000, 0x100000, CRC(ef652e0c) SHA1(6dd994a15ced31d1bbd1a3b0e9d8d86eca33e217) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layers 0 + 1 */
	ROM_LOAD( "u33.bin",  0x000000, 0x200000, CRC(54494e6b) SHA1(f5d090d2d34d908b56b53a246def194929eba990) )

	ROM_REGION( 0x100000, "ymsnd", 0 )	/* ADPCM Samples */
	ROM_LOAD( "u56.bin",  0x000000, 0x100000, CRC(9e07104d) SHA1(3bc54cb755bb3194197706965b532d62b48c4d12) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )	/* DELTA-T Samples */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(e187ed4f) SHA1(05060723d89b1d05714447a14b5f5888ff3c2306) )

	ROM_REGION( 0x040000, "spritelut", 0 )	/* Sprites LUT */
	ROM_LOAD( "u3.bin",  0x000000, 0x040000, CRC(0905aeb2) SHA1(8cca09f7dfe3f804e77515f7b1b1bdbeb7bb3d80) )

ROM_END


ROM_START( btlkroad )

	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "4-u46.bin", 0x000000, 0x040000, CRC(8a7a28b4) SHA1(f7197be673dfd0ddf46998af81792b81d8fe9fbf) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "5-u39.bin", 0x000002, 0x040000, CRC(933561fa) SHA1(f6f3b1e14b1cfeca26ef8260ac4771dc1531c357) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "3-u71.bin", 0x00000, 0x20000, CRC(22411fab) SHA1(1094cb51712e40ae65d0082b408572bdec06ae8b) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x700000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u14.bin",  0x000000, 0x200000, CRC(282d89c3) SHA1(3b4b17f4a37efa2f7e232488aaba7c77d10c84d2) )
	ROM_LOAD( "u24.bin",  0x200000, 0x200000, CRC(bbe9d3d1) SHA1(9da0b0b993e8271a8119e9c2f602e52325983f79) )
	ROM_LOAD( "u15.bin",  0x400000, 0x200000, CRC(d4d1b07c) SHA1(232109db8f6e137fbc8826f38a96057067cb19dc) )
//  ROM_LOAD( "u25.bin",  0x600000, 0x100000  NOT PRESENT

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layers 0 + 1 */
	ROM_LOAD( "u33.bin",  0x000000, 0x200000, CRC(4c8577f1) SHA1(d27043514632954a06667ac63f4a4e4a31870511) )

	ROM_REGION( 0x100000, "ymsnd", 0 )	/* ADPCM Samples */
	ROM_LOAD( "u56.bin",  0x000000, 0x100000, CRC(51d73682) SHA1(562038d08e9a4389ffa39f3a659b2a29b94dc156) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )	/* DELTA-T Samples */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(0f33049f) SHA1(ca4fd5f3906685ace1af40b75f5678231d7324e8) )

	ROM_REGION( 0x040000, "spritelut", 0 )	/* Sprites LUT */
	ROM_LOAD( "u3.bin",  0x000000, 0x040000, CRC(30d541ed) SHA1(6f7fb5f5ecbce7c086185392de164ebb6887e780) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "tibpal16l8.u69", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "tibpal16l8.u19", 0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */

ROM_END

/***************************************************************************

                            Strikers 1945 (Japan, unprotected)

Board:  SH403 (Similiar to KA302C)
CPU:    MC68EC020FG16
Sound:  LZ8420M (Z80 core) + YMF286-K?
OSC:    16.000, 14.31818 MHz

Chips:  PS2001B
        PS3103
        PS3204
        PS3305

***************************************************************************/

ROM_START( s1945jn )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "1-u46.bin", 0x000000, 0x080000, CRC(45fa8086) SHA1(f1753b9420596f4b828c77e877a044ba5fb01b28) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "2-u39.bin", 0x000002, 0x080000, CRC(0152ab8c) SHA1(2aef4cb88341b35f20bb551716f1e5ac2731e9ba) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "3-u71.bin", 0x00000, 0x20000, CRC(e3e366bd) SHA1(1f5b5909745802e263a896265ea365df76d3eaa5) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x800000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u20.bin",  0x000000, 0x200000, CRC(28a27fee) SHA1(913f3bc4d0c6fb6b776a020c8099bf96f16fd06f) )
	ROM_LOAD( "u22.bin",  0x200000, 0x200000, CRC(ca152a32) SHA1(63efee83cb5982c77ca473288b3d1a96b89e6388) )
	ROM_LOAD( "u21.bin",  0x400000, 0x200000, CRC(c5d60ea9) SHA1(e5ce90788211c856172e5323b01b2c7ab3d3fe50) )
	ROM_LOAD( "u23.bin",  0x600000, 0x200000, CRC(48710332) SHA1(db38b732a09b31ce55a96ec62987baae9b7a00c1) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 0 + 1 */
	ROM_LOAD( "u34.bin",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x100000, "ymsnd", 0 )	/* ADPCM Samples */
	ROM_LOAD( "u56.bin",  0x000000, 0x100000, CRC(fe1312c2) SHA1(8339a96a0885518d6e22cb3bdb9c2f82d011d86d) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )	/* DELTA-T Samples */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(a44a4a9b) SHA1(5378256752d709daed0b5f4199deebbcffe84e10) )

	ROM_REGION( 0x040000, "spritelut", 0 )	/* */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(dee22654) SHA1(5df05b0029ff7b1f7f04b41da7823d2aa8034bd2) )
ROM_END

/* closely based on s1945jn set, unsurprising because it's unprotected */
ROM_START( s1945bl )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_BYTE( "27c010-1", 0x000000, 0x020000, CRC(d3361536) SHA1(430df1c98645603c17333222834d344efd4fb584) ) // 1-u46.bin    [odd 1/2]  99.797821%
	ROM_LOAD32_BYTE( "27c010-2", 0x000001, 0x020000, CRC(1d1916b1) SHA1(4e200454c16d0bd45c4146ee41902a811a55c008) ) // 1-u46.bin    [even 1/2] 99.793243%
	ROM_LOAD32_BYTE( "27c010-3", 0x000002, 0x020000, CRC(391e0387) SHA1(5c5c737629a450e8d07b088ad50280dae57aeded) ) // 2-u39.bin    [odd 1/2]  99.749756%
	ROM_LOAD32_BYTE( "27c010-4", 0x000003, 0x020000, CRC(2aebcf6b) SHA1(2aea1c5edc006f70c21d84b581a48082ec111f6a) ) // 2-u39.bin    [even 1/2] 99.743652%

	ROM_REGION( 0x800000, "gfx1", 0 )	/* Sprites */
	// same content as original sets, alt rom layout
	ROM_LOAD16_WORD_SWAP( "rv27c3200.m4",  0x000000, 0x400000, CRC(70c8f72e) SHA1(90d25f4ecd6bfe72b51713099625f643b12aa674) )
	ROM_LOAD16_WORD_SWAP( "rv27c3200.m3",  0x400000, 0x400000, CRC(0dec2a8d) SHA1(b2f3143f2be50c825b61d5218cec26ba8ed1f07e) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 0 + 1 */
	ROM_LOAD( "rv27c1600.m1",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x100000, "oki", 0 )	/* OKI Samples */
	ROM_LOAD( "rv27c040.m6",  0x000000, 0x080000, CRC(c22e5b65) SHA1(d807bd7c136d6b51f54258b44ebf3eecbd5b35fa) )

	ROM_REGION( 0x040000, "spritelut", 0 )	/* */
	ROM_LOAD16_BYTE( "27c010-b",  0x000000, 0x020000, CRC(e38d5ab7) SHA1(73a708ebc305cb6297efd3296da23c87898e805e) ) // u1.bin       [even]     IDENTICAL
	ROM_LOAD16_BYTE( "27c010-a",  0x000001, 0x020000, CRC(cb8c65ec) SHA1(a55c5c5067b50a1243e7ba60fa1f9569bfed5de8) ) // u1.bin       [odd]      99.999237%

	ROM_REGION( 0x080000, "unknown", 0 )	/* unknown - matches Semicom's Dream World */
	ROM_LOAD( "27c512",  0x000000, 0x010000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END

/***************************************************************************

Strikers 1945
1995, Psikyo

PCB Layout
----------

SH404
|----------------------------------------------------------|
|HA1388  2058D   62256 6264  62256  16MHz PAL1 * 3.U63 U61 |
|       YAC513-M 62256 62256 6264   14.31818MHz  33.8688MHz|
|  VOL                                                     |
|                         |------|62256 |-------||-------| |
|      4MHz   LH5168      |PS3204|62256 |LZ8420M||YMF278B| |
|     16C57   LH5168      |------|      |-------||-------| |
|         PAL2                                             |
|                                                          |
|J                                          |--------|     |
|A           |------| 5.U41   62256  62256  |        |     |
|M           |68020 | 4.U40   62256  62256  | PS3103 | U34 |
|M           |------|                       |        |     |
|A  |------|                                |--------|     |
|   |PS3305|                                               |
|   |------|              |--------|                       |
|                         |        |      U23 U22 U21 U20  |
| MB3771                  |PS2001B |                       |
|    6264   62256 62256   |        |                       |
|    6264   62256 62256   |--------|                       |
|                                                          |
|DIP1                     6116  6116                       |
|DIP2                     6116  6116        LH5168     U1  |
|----------------------------------------------------------|
Notes:
      68020    - Motorola MC68EC020 CPU (QFP100, running at 16.000MHz)
      PS3305   - Psikyo Custom (QFP100)
      PS3204   - Psikyo Custom (QFP100)
      PS2001B  - Psikyo Custom (QFP160)
      PS3103   - Psikyo Custom (QFP160)
      6116     - 2k x8 SRAM (x4, DIP24)
      LH5168   - Sharp LH5168 2k x8 SRAM (x3, DIP24)
      62256    - NKK N341256 32k x8 SRAM (x14, DIP28)
      6264     - ISSI IS61C64 8k x8 SRAM (x4, DIP28)
      MB3771   - Fujitsu MB3771 Master Reset IC (DIP8)
      DIP1     - 8 position DIP Switch
      DIP2     - 8 position DIP Switch
      PAL1     - AMD PALCE 16V8H (DIP20, stamped '4041')
      PAL2     - AMD PALCE 16V8H (DIP20, stamped '4040')
      YMF278B  - Yamaha YMF278B-F Sound Chip, running at 33.8688MHz (QFP64)
      3.U63    - Macronix MX27C1000 128k x8 EPROM (Sound Program, DIP32, labelled '3')
      4.U40    - AMD AM27C2048 256k x16 EPROM (Main Program, DIP40, labelled '4' & '5')
      5.U41    /
      16C57    - Microchip PIC16C57 Microcontroller (DIP24, running at 4.000MHz, labelled '1')
      YAC513   - Yamaha YAC513-M D/A Converter (SOIC16)
      LZ8420M  - Sharp LZ8420M Z80-core CPU (QFP64, running at 4.000MHz)
      HA1388   - Hitachi HA1388 Sound Amp
      2058D    - JRC 2058 Op Amp
      U61, U34,\
      U20, U21,- 16M MASKROM (SOP44)
      U22, U23 /
      U1       - 4M MASKROM (SOP44)
      VOL      - Master Volume Potentiometer
      *        - Unpopulated position for 16M SOP44 MASKROM


CPU:    MC68EC020FG16
OSC:    16.000MHz
        14.3181MHz
        33.8688MHz (YMF)
        4.000MHz (PIC)

1-U59   security (PIC16C57; not dumped)

***************************************************************************/

ROM_START( s1945 )

	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "2s.u40", 0x000000, 0x040000, CRC(9b10062a) SHA1(cf963bba157422b659d8d64b4493eb7d69cd07b7) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "3s.u41", 0x000002, 0x040000, CRC(f87e871a) SHA1(567167c7fcfb622f78e211d74b04060c3d29d6b7) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "3-u63.bin", 0x00000, 0x20000, CRC(42d40ae1) SHA1(530a5a3f78ac489b84a631ea6ce21010a4f4d31b) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x000100, "cpu2", 0 )		/* MCU? */
	ROM_LOAD( "4-u59.bin", 0x00000, 0x00100, NO_DUMP )

	ROM_REGION( 0x800000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u20.bin",  0x000000, 0x200000, CRC(28a27fee) SHA1(913f3bc4d0c6fb6b776a020c8099bf96f16fd06f) )
	ROM_LOAD( "u22.bin",  0x200000, 0x200000, CRC(ca152a32) SHA1(63efee83cb5982c77ca473288b3d1a96b89e6388) )
	ROM_LOAD( "u21.bin",  0x400000, 0x200000, CRC(c5d60ea9) SHA1(e5ce90788211c856172e5323b01b2c7ab3d3fe50) )
	ROM_LOAD( "u23.bin",  0x600000, 0x200000, CRC(48710332) SHA1(db38b732a09b31ce55a96ec62987baae9b7a00c1) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 0 + 1 */
	ROM_LOAD( "u34.bin",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x200000, "ymf", 0 )	/* Samples */
	ROM_LOAD( "u61.bin",  0x000000, 0x200000, CRC(a839cf47) SHA1(e179eb505c80d5bb3ccd9e228f2cf428c62b72ee) )	// 8 bit signed pcm (16KHz)

	ROM_REGION( 0x040000, "spritelut", 0 )	/* */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(dee22654) SHA1(5df05b0029ff7b1f7f04b41da7823d2aa8034bd2) )

ROM_END

ROM_START( s1945a )

	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "4-u40.bin", 0x000000, 0x040000, CRC(29ffc217) SHA1(12dc3cb32253c3908f4c440c627a0e1b32ee7cac) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "5-u41.bin", 0x000002, 0x040000, CRC(c3d3fb64) SHA1(4388586bc0a6f3d62366b3c38b8b23f8a03dbf15) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "3-u63.bin", 0x00000, 0x20000, CRC(42d40ae1) SHA1(530a5a3f78ac489b84a631ea6ce21010a4f4d31b) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x000100, "cpu2", 0 )		/* MCU? */
	ROM_LOAD( "4-u59.bin", 0x00000, 0x00100, NO_DUMP )

	ROM_REGION( 0x800000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u20.bin",  0x000000, 0x200000, CRC(28a27fee) SHA1(913f3bc4d0c6fb6b776a020c8099bf96f16fd06f) )
	ROM_LOAD( "u22.bin",  0x200000, 0x200000, CRC(ca152a32) SHA1(63efee83cb5982c77ca473288b3d1a96b89e6388) )
	ROM_LOAD( "u21.bin",  0x400000, 0x200000, CRC(c5d60ea9) SHA1(e5ce90788211c856172e5323b01b2c7ab3d3fe50) )
	ROM_LOAD( "u23.bin",  0x600000, 0x200000, CRC(48710332) SHA1(db38b732a09b31ce55a96ec62987baae9b7a00c1) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 0 + 1 */
	ROM_LOAD( "u34.bin",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x200000, "ymf", 0 )	/* Samples */
	ROM_LOAD( "u61.bin",  0x000000, 0x200000, CRC(a839cf47) SHA1(e179eb505c80d5bb3ccd9e228f2cf428c62b72ee) )	// 8 bit signed pcm (16KHz)

	ROM_REGION( 0x040000, "spritelut", 0 )	/* */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(dee22654) SHA1(5df05b0029ff7b1f7f04b41da7823d2aa8034bd2) )

ROM_END

ROM_START( s1945j )

	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "1-u40.bin", 0x000000, 0x040000, CRC(c00eb012) SHA1(080dae010ca83548ebdb3324585d15e48baf0541) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "2-u41.bin", 0x000002, 0x040000, CRC(3f5a134b) SHA1(18bb3bb1e1adadcf522795f5cf7d4dc5a5dd1f30) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "3-u63.bin", 0x00000, 0x20000, CRC(42d40ae1) SHA1(530a5a3f78ac489b84a631ea6ce21010a4f4d31b) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x000100, "cpu2", 0 )		/* MCU */
	ROM_LOAD( "4-u59.bin", 0x00000, 0x00100, NO_DUMP )

	ROM_REGION( 0x800000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u20.bin",  0x000000, 0x200000, CRC(28a27fee) SHA1(913f3bc4d0c6fb6b776a020c8099bf96f16fd06f) )
	ROM_LOAD( "u22.bin",  0x200000, 0x200000, CRC(ca152a32) SHA1(63efee83cb5982c77ca473288b3d1a96b89e6388) )
	ROM_LOAD( "u21.bin",  0x400000, 0x200000, CRC(c5d60ea9) SHA1(e5ce90788211c856172e5323b01b2c7ab3d3fe50) )
	ROM_LOAD( "u23.bin",  0x600000, 0x200000, CRC(48710332) SHA1(db38b732a09b31ce55a96ec62987baae9b7a00c1) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 0 + 1 */
	ROM_LOAD( "u34.bin",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x200000, "ymf", 0 )	/* Samples */
	ROM_LOAD( "u61.bin",  0x000000, 0x200000, CRC(a839cf47) SHA1(e179eb505c80d5bb3ccd9e228f2cf428c62b72ee) )	// 8 bit signed pcm (16KHz)

	ROM_REGION( 0x040000, "spritelut", 0 )	/* */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(dee22654) SHA1(5df05b0029ff7b1f7f04b41da7823d2aa8034bd2) )

ROM_END

ROM_START( s1945k ) /* Same MCU as the current parent set, region dip has no effect on this set */

	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "10.u40", 0x000000, 0x040000, CRC(5a32af36) SHA1(2eada37fd043c097a11bcf4e3e0bebb473bbc0df) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "9.u41",  0x000002, 0x040000, CRC(29cc6d7d) SHA1(aeee9e88922c25c75885483d115e064c6b71540b) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "3-u63.bin", 0x00000, 0x20000, CRC(42d40ae1) SHA1(530a5a3f78ac489b84a631ea6ce21010a4f4d31b) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x000100, "cpu2", 0 )		/* MCU */
	ROM_LOAD( "4-u59.bin", 0x00000, 0x00100, NO_DUMP )

	ROM_REGION( 0x800000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u20.bin",  0x000000, 0x200000, CRC(28a27fee) SHA1(913f3bc4d0c6fb6b776a020c8099bf96f16fd06f) )
	ROM_LOAD( "u22.bin",  0x200000, 0x200000, CRC(ca152a32) SHA1(63efee83cb5982c77ca473288b3d1a96b89e6388) )
	ROM_LOAD( "u21.bin",  0x400000, 0x200000, CRC(c5d60ea9) SHA1(e5ce90788211c856172e5323b01b2c7ab3d3fe50) )
	ROM_LOAD( "u23.bin",  0x600000, 0x200000, CRC(48710332) SHA1(db38b732a09b31ce55a96ec62987baae9b7a00c1) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 0 + 1 */
	ROM_LOAD( "u34.bin",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x200000, "ymf", 0 )	/* Samples */
	ROM_LOAD( "u61.bin",  0x000000, 0x200000, CRC(a839cf47) SHA1(e179eb505c80d5bb3ccd9e228f2cf428c62b72ee) )	// 8 bit signed pcm (16KHz)

	ROM_REGION( 0x040000, "spritelut", 0 )	/* */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(dee22654) SHA1(5df05b0029ff7b1f7f04b41da7823d2aa8034bd2) )

ROM_END


/***************************************************************************

                                Tengai (World) / Sengoku Blade (Japan)

Board:  SH404
CPU:    MC68EC020FG16
Sound:  LZ8420M (Z80 core)
        YMF278B-F
OSC:    16.000MHz
        14.3181MHz
        33.8688MHz (YMF)
        4.000MHz (PIC)
Chips:  PS2001B
        PS3103
        PS3204
        PS3305

4-U59      security (PIC16C57; not dumped)

***************************************************************************/

ROM_START( tengai )

	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "5-u40.bin", 0x000000, 0x080000, CRC(90088195) SHA1(8ec48d581ecd14b3dad36edc65d5a273324cf3c1) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "4-u41.bin", 0x000002, 0x080000, CRC(0d53196c) SHA1(454bb4695b13ce44ca5dac7c6d4142a8b9afa798) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "1-u63.bin", 0x00000, 0x20000, CRC(2025e387) SHA1(334b0eb3b416d46ccaadff3eee6f1abba63285fb) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x000100, "cpu2", 0 )		/* MCU */
	ROM_LOAD( "4-u59.bin", 0x00000, 0x00100, NO_DUMP )

	ROM_REGION( 0x600000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_WORD_SWAP( "u20.bin",  0x000000, 0x200000, CRC(ed42ef73) SHA1(74693fcc83a2654ddb18fd513d528033863d6116) )
	ROM_LOAD16_WORD_SWAP( "u22.bin",  0x200000, 0x200000, CRC(8d21caee) SHA1(2a68af8b2be2158dcb152c434e91a75871478d41) )
	ROM_LOAD16_WORD_SWAP( "u21.bin",  0x400000, 0x200000, CRC(efe34eed) SHA1(7891495b443a5acc7b2f17fe694584f6cb0afacc) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layer 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u34.bin",  0x000000, 0x400000, CRC(2a2e2eeb) SHA1(f1d99353c0affc5c908985e6f2a5724e5223cccc) ) /* four banks of 0x100000 */

	ROM_REGION( 0x400000, "ymf", 0 )	/* Samples */
	ROM_LOAD( "u61.bin",  0x000000, 0x200000, CRC(a63633c5) SHA1(89e75a40518926ebcc7d88dea86c01ba0bb496e5) )	// 8 bit signed pcm (16KHz)
	ROM_LOAD( "u62.bin",  0x200000, 0x200000, CRC(3ad0c357) SHA1(35f78cfa2eafa93ab96b24e336f569ee84af06b6) )

	ROM_REGION( 0x040000, "spritelut", 0 )	/* Sprites LUT */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(681d7d55) SHA1(b0b28471440d747adbc4d22d1918f89f6ede1615) )

ROM_END

ROM_START( tengaij )

	ROM_REGION( 0x100000, "maincpu", 0 )		/* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "2-u40.bin", 0x000000, 0x080000, CRC(ab6fe58a) SHA1(6687a3af192b3eab60d75ca286ebb8e0636297b5) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "3-u41.bin", 0x000002, 0x080000, CRC(02e42e39) SHA1(6cdb7b1cebab50c0a44cd60cd437f0e878ccac5c) ) // 3&2

	ROM_REGION( 0x030000, "audiocpu", 0 )		/* Sound CPU Code */
	ROM_LOAD( "1-u63.bin", 0x00000, 0x20000, CRC(2025e387) SHA1(334b0eb3b416d46ccaadff3eee6f1abba63285fb) )
	ROM_RELOAD(            0x10000, 0x20000             )

	ROM_REGION( 0x000100, "cpu2", 0 )		/* MCU */
	ROM_LOAD( "4-u59.bin", 0x00000, 0x00100, NO_DUMP )

	ROM_REGION( 0x600000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_WORD_SWAP( "u20.bin",  0x000000, 0x200000, CRC(ed42ef73) SHA1(74693fcc83a2654ddb18fd513d528033863d6116) )
	ROM_LOAD16_WORD_SWAP( "u22.bin",  0x200000, 0x200000, CRC(8d21caee) SHA1(2a68af8b2be2158dcb152c434e91a75871478d41) )
	ROM_LOAD16_WORD_SWAP( "u21.bin",  0x400000, 0x200000, CRC(efe34eed) SHA1(7891495b443a5acc7b2f17fe694584f6cb0afacc) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layer 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u34.bin",  0x000000, 0x400000, CRC(2a2e2eeb) SHA1(f1d99353c0affc5c908985e6f2a5724e5223cccc) ) /* four banks of 0x100000 */

	ROM_REGION( 0x400000, "ymf", 0 )	/* Samples */
	ROM_LOAD( "u61.bin",  0x000000, 0x200000, CRC(a63633c5) SHA1(89e75a40518926ebcc7d88dea86c01ba0bb496e5) )	// 8 bit signed pcm (16KHz)
	ROM_LOAD( "u62.bin",  0x200000, 0x200000, CRC(3ad0c357) SHA1(35f78cfa2eafa93ab96b24e336f569ee84af06b6) )

	ROM_REGION( 0x040000, "spritelut", 0 )	/* Sprites LUT */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(681d7d55) SHA1(b0b28471440d747adbc4d22d1918f89f6ede1615) )

ROM_END

/***************************************************************************


                                Driver Initialization


***************************************************************************/

static DRIVER_INIT( sngkace )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	{
		UINT8 *RAM = memory_region(machine, "ymsnd");
		int len = memory_region_length(machine, "ymsnd");
		int i;

		/* Bit 6&7 of the samples are swapped. Naughty, naughty... */
		for (i = 0; i < len; i++)
		{
			int x = RAM[i];
			RAM[i] = ((x & 0x40) << 1) | ((x & 0x80) >> 1) | (x & 0x3f);
		}
	}

	/* input ports */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00000, 0xc0000b, 0, 0, sngkace_input_r);

	/* sound latch */
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00010, 0xc00013, 0, 0, psikyo_soundlatch_w);

	state->ka302c_banking = 0; // SH201B doesn't have any gfx banking

	/* setup audiocpu banks */
	memory_configure_bank(machine, "bank1", 0, 4, memory_region(machine, "audiocpu") + 0x10000, 0x8000);

	/* Enable other regions */
#if 0
	if (!strcmp(machine->gamedrv->name,"sngkace"))
	{
		UINT8 *ROM	=	memory_region(machine, "maincpu");
		ROM[0x995] = 0x4e;
		ROM[0x994] = 0x71;
		ROM[0x997] = 0x4e;
		ROM[0x996] = 0x71;

	}
#endif
}

static void s1945_mcu_init( running_machine *machine )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;
	state->s1945_mcu_direction = 0x00;
	state->s1945_mcu_inlatch = 0xff;
	state->s1945_mcu_latch1 = 0xff;
	state->s1945_mcu_latch2 = 0xff;
	state->s1945_mcu_latching = 0x5;
	state->s1945_mcu_control = 0xff;
	state->s1945_mcu_index = 0;
	state->s1945_mcu_mode = 0;
	state->s1945_mcu_bctrl = 0x00;

	state_save_register_global(machine, state->s1945_mcu_direction);
	state_save_register_global(machine, state->s1945_mcu_inlatch);
	state_save_register_global(machine, state->s1945_mcu_latch1);
	state_save_register_global(machine, state->s1945_mcu_latch2);
	state_save_register_global(machine, state->s1945_mcu_latching);
	state_save_register_global(machine, state->s1945_mcu_control);
	state_save_register_global(machine, state->s1945_mcu_index);
	state_save_register_global(machine, state->s1945_mcu_mode);
	state_save_register_global(machine, state->s1945_mcu_bctrl);
}

static DRIVER_INIT( tengai )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	/* input ports */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00000, 0xc0000b, 0, 0, s1945_input_r);

	/* sound latch */
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00010, 0xc00013, 0, 0, s1945_soundlatch_w);

	/* protection */
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00004, 0xc0000b, 0, 0, s1945_mcu_w);

	s1945_mcu_init(machine);
	state->s1945_mcu_table = 0;

	state->ka302c_banking = 0; // Banking is controlled by mcu

	/* setup audiocpu banks */
	/* The banked rom is seen at 8200-ffff, so the last 0x200 bytes of the rom not reachable. */
	memory_configure_bank(machine, "bank1", 0, 4, memory_region(machine, "audiocpu") + 0x10000 + 0x200, 0x8000);
}

static DRIVER_INIT( gunbird )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	/* input ports */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00000, 0xc0000b, 0, 0, gunbird_input_r);

	/* sound latch */
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00010, 0xc00013, 0, 0, psikyo_soundlatch_w);

	state->ka302c_banking = 1;

	/* setup audiocpu banks */
	/* The banked rom is seen at 8200-ffff, so the last 0x200 bytes of the rom not reachable. */
	memory_configure_bank(machine, "bank1", 0, 4, memory_region(machine, "audiocpu") + 0x10000 + 0x200, 0x8000);
}


static DRIVER_INIT( s1945 )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	/* input ports */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00000, 0xc0000b, 0, 0, s1945_input_r);

	/* sound latch */
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00010, 0xc00013, 0, 0, s1945_soundlatch_w);

	/* protection and tile bank switching */
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00004, 0xc0000b, 0, 0, s1945_mcu_w);

	s1945_mcu_init(machine);
	state->s1945_mcu_table = s1945_table;

	state->ka302c_banking = 0; // Banking is controlled by mcu

	/* setup audiocpu banks */
	/* The banked rom is seen at 8200-ffff, so the last 0x200 bytes of the rom not reachable. */
	memory_configure_bank(machine, "bank1", 0, 4, memory_region(machine, "audiocpu") + 0x10000 + 0x200, 0x8000);
}

static DRIVER_INIT( s1945a )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	/* input ports */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00000, 0xc0000b, 0, 0, s1945_input_r);

	/* sound latch */
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00010, 0xc00013, 0, 0, s1945_soundlatch_w);

	/* protection and tile bank switching */
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00004, 0xc0000b, 0, 0, s1945_mcu_w);

	s1945_mcu_init(machine);
	state->s1945_mcu_table = s1945a_table;

	state->ka302c_banking = 0; // Banking is controlled by mcu

	/* setup audiocpu banks */
	/* The banked rom is seen at 8200-ffff, so the last 0x200 bytes of the rom not reachable. */
	memory_configure_bank(machine, "bank1", 0, 4, memory_region(machine, "audiocpu") + 0x10000 + 0x200, 0x8000);
}

static DRIVER_INIT( s1945j )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	/* input ports*/
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00000, 0xc0000b, 0, 0, s1945_input_r);

	/* sound latch */
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00010, 0xc00013, 0, 0, s1945_soundlatch_w);

	/* protection and tile bank switching */
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00004, 0xc0000b, 0, 0, s1945_mcu_w);

	s1945_mcu_init(machine);
	state->s1945_mcu_table = s1945j_table;

	state->ka302c_banking = 0; // Banking is controlled by mcu

	/* setup audiocpu banks */
	/* The banked rom is seen at 8200-ffff, so the last 0x200 bytes of the rom not reachable. */
	memory_configure_bank(machine, "bank1", 0, 4, memory_region(machine, "audiocpu") + 0x10000 + 0x200, 0x8000);
}

static DRIVER_INIT( s1945jn )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	/* input ports */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00000, 0xc0000b, 0, 0, gunbird_input_r);

	/* sound latch */
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00010, 0xc00013, 0, 0, s1945_soundlatch_w);

	state->ka302c_banking = 1;

	/* setup audiocpu banks */
	/* The banked rom is seen at 8200-ffff, so the last 0x200 bytes of the rom not reachable. */
	memory_configure_bank(machine, "bank1", 0, 4, memory_region(machine, "audiocpu") + 0x10000 + 0x200, 0x8000);
}

static DRIVER_INIT( s1945bl )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	/* input ports */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00000, 0xc0000b, 0, 0, gunbird_input_r);

	/* sound latch */
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00010, 0xc00013, 0, 0, s1945_soundlatch_w);

	state->ka302c_banking = 1;

	memory_configure_bank(machine, "okibank", 0, 4, memory_region(machine, "oki") + 0x30000, 0x10000);
	memory_set_bank(machine, "okibank", 0);
}


/***************************************************************************


                                Game Drivers


***************************************************************************/

/* Working Games */
GAME( 1993, samuraia, 0,        sngkace,  samuraia, sngkace,  ROT270, "Psikyo", "Samurai Aces (World)", GAME_SUPPORTS_SAVE ) // Banpresto?
GAME( 1993, sngkace,  samuraia, sngkace,  sngkace,  sngkace,  ROT270, "Psikyo", "Sengoku Ace (Japan)", GAME_SUPPORTS_SAVE ) // Banpresto?
GAME( 1994, gunbird,  0,        gunbird,  gunbird,  gunbird,  ROT270, "Psikyo", "Gunbird (World)", GAME_SUPPORTS_SAVE )
GAME( 1994, gunbirdk, gunbird,  gunbird,  gunbirdj, gunbird,  ROT270, "Psikyo", "Gunbird (Korea)", GAME_SUPPORTS_SAVE )
GAME( 1994, gunbirdj, gunbird,  gunbird,  gunbirdj, gunbird,  ROT270, "Psikyo", "Gunbird (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1994, btlkroad, 0,        gunbird,  btlkroad, gunbird,  ROT0,   "Psikyo", "Battle K-Road", GAME_SUPPORTS_SAVE )
GAME( 1995, s1945,    0,        s1945,    s1945,    s1945,    ROT270, "Psikyo", "Strikers 1945 (World)", GAME_SUPPORTS_SAVE )
GAME( 1995, s1945a,   s1945,    s1945,    s1945a,   s1945a,   ROT270, "Psikyo", "Strikers 1945 (Japan / World)", GAME_SUPPORTS_SAVE ) // Region dip - 0x0f=Japan, anything else=World
GAME( 1995, s1945j,   s1945,    s1945,    s1945j,   s1945j,   ROT270, "Psikyo", "Strikers 1945 (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1995, s1945jn,  s1945,    gunbird,  s1945j,   s1945jn,  ROT270, "Psikyo", "Strikers 1945 (Japan, unprotected)", GAME_SUPPORTS_SAVE )
GAME( 1995, s1945k,   s1945,    s1945,    s1945j,   s1945,    ROT270, "Psikyo", "Strikers 1945 (Korea)", GAME_SUPPORTS_SAVE )
GAME( 1995, s1945bl,  s1945,    s1945bl,  s1945bl,  s1945bl,  ROT270, "bootleg","Strikers 1945 (Hong Kong, bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1996, tengai,   0,        s1945,    tengai,   tengai,   ROT0,   "Psikyo", "Tengai (World)", GAME_SUPPORTS_SAVE )
GAME( 1996, tengaij,  tengai,   s1945,    tengaij,  tengai,   ROT0,   "Psikyo", "Sengoku Blade: Sengoku Ace Episode II / Tengai", GAME_SUPPORTS_SAVE ) // Region dip - 0x0f=Japan, anything else=World
