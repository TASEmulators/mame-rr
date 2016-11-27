#define MW_DEBUG     0

/**************************************************************************
 * Mystic Warrior (c) 1993 Konami
 * Metamorphic Force (c) 1993 Konami
 * Violent Storm (c) 1993 Konami
 * Martial Champion (c) 1993 Konami
 * Gaiapolis (c) 1993 Konami
 * Ultimate Battler Dadandarn!! (c) 1993 Konami
 *
 * Driver by R. Belmont, Phil Stroffolino, Acho Tang, and Nicola Salmoria.
 * Assists from Olivier Galibert, Brian Troha, The Guru, and Yasuhiro Ogawa.
 *
 * These games are the "pre-GX" boards, combining features of the previous
 * line of hardware begun with Xexex and those of the future 32-bit System
 * GX (notably 5 bit per pixel graphics, the powerful K055555 mixer/priority
 * encoder, and K054338 alpha blend engine from System GX are used).
 *
 * Game status:
 * - All games are playable with sound and correct colors.
 * - Metamorphic Force's intro needs alpha blended sprites.
 */

#include "emu.h"
#include "deprecat.h"

#include "video/konamiic.h"
#include "includes/konamigx.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "sound/k054539.h"
#include "includes/konamipt.h"

VIDEO_START(gaiapols);
VIDEO_START(dadandrn);
VIDEO_START(viostorm);
VIDEO_START(metamrph);
VIDEO_START(martchmp);
VIDEO_START(mystwarr);
VIDEO_UPDATE(dadandrn);
VIDEO_UPDATE(mystwarr);
VIDEO_UPDATE(metamrph);
VIDEO_UPDATE(martchmp);

WRITE16_HANDLER(ddd_053936_enable_w);
WRITE16_HANDLER(ddd_053936_clip_w);
READ16_HANDLER(gai_053936_tilerom_0_r);
READ16_HANDLER(ddd_053936_tilerom_0_r);
READ16_HANDLER(ddd_053936_tilerom_1_r);
READ16_HANDLER(gai_053936_tilerom_2_r);
READ16_HANDLER(ddd_053936_tilerom_2_r);

static UINT16 *gx_workram;

static UINT8 mw_irq_control;

static const eeprom_interface eeprom_intf =
{
	7,			/* address bits */
	8,			/* data bits */
	"011000",		/*  read command */
	"011100",		/* write command */
	"0100100000000",/* erase command */
	"0100000000000",/* lock command */
	"0100110000000" /* unlock command */
};

/* Gaiapolis and Polygonet Commanders use the ER5911,
   but the command formats are slightly different.  Why? */
static const eeprom_interface gaia_eeprom_intf =
{
	7,			/* address bits */
	8,			/* data bits */
	"011000",		/*  read command */
	"010100",		/* write command */
	"0100100000000",/* erase command */
	"0100000000000",/* lock command */
	"0100110000000" /* unlock command */
};

static READ16_HANDLER( eeprom_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return input_port_read(space->machine, "IN1");
	}

//  logerror("msb access to eeprom port\n");

	return 0;
}

static WRITE16_HANDLER( mweeprom_w )
{
	if (ACCESSING_BITS_8_15)
	{
		input_port_write(space->machine, "EEPROMOUT", data, 0xffff);
	}

//  logerror("unknown LSB write %x to eeprom\n", data);

}

static READ16_HANDLER( dddeeprom_r )
{
	if (ACCESSING_BITS_8_15)
	{
		return input_port_read(space->machine, "IN1") << 8;
	}

	return input_port_read(space->machine, "P2");
}

static WRITE16_HANDLER( mmeeprom_w )
{
	if (ACCESSING_BITS_0_7)
	{
		input_port_write(space->machine, "EEPROMOUT", data, 0xff);
	}
}


/**********************************************************************************/
/* IRQ controllers */

static INTERRUPT_GEN(mystwarr_interrupt)
{
	if (!(mw_irq_control & 0x01)) return;

	switch (cpu_getiloops(device))
	{
		case 0:
			cpu_set_input_line(device, M68K_IRQ_2, HOLD_LINE);
		break;

		case 1:
			cpu_set_input_line(device, M68K_IRQ_4, HOLD_LINE);
		break;

		case 2:
			cpu_set_input_line(device, M68K_IRQ_6, HOLD_LINE);
		break;
	}
}

static INTERRUPT_GEN(metamrph_interrupt)
{
	switch (cpu_getiloops(device))
	{
		case 0:
			cpu_set_input_line(device, M68K_IRQ_4, HOLD_LINE);
		break;

		case 15:
			cpu_set_input_line(device, M68K_IRQ_6, HOLD_LINE);
		break;

		case 39:
			if (K053246_is_IRQ_enabled()) cpu_set_input_line(device, M68K_IRQ_5, HOLD_LINE);
		break;
	}
}

static INTERRUPT_GEN(mchamp_interrupt)
{
	if (!(mw_irq_control & 0x02)) return;

	switch (cpu_getiloops(device))
	{
		case 0:
			if (K053246_is_IRQ_enabled()) cpu_set_input_line(device, M68K_IRQ_6, HOLD_LINE);
		break;

		case 1:
			cpu_set_input_line(device, M68K_IRQ_2, HOLD_LINE);
		break;
	}
}

static INTERRUPT_GEN(ddd_interrupt)
{
	cpu_set_input_line(device, M68K_IRQ_5, HOLD_LINE);
}


/**********************************************************************************/

static WRITE16_HANDLER( sound_cmd1_w )
{
	soundlatch_w(space, 0, data&0xff);
}

static WRITE16_HANDLER( sound_cmd1_msb_w )
{
	soundlatch_w(space, 0, data>>8);
}

static WRITE16_HANDLER( sound_cmd2_w )
{
	soundlatch2_w(space, 0, data&0xff);
	return;
}

static WRITE16_HANDLER( sound_cmd2_msb_w )
{
	soundlatch2_w(space, 0, data>>8);
	return;
}

static WRITE16_HANDLER( sound_irq_w )
{
	cputag_set_input_line(space->machine, "soundcpu", 0, HOLD_LINE);
}

static READ16_HANDLER( sound_status_r )
{
	int latch = soundlatch3_r(space,0);

	if ((latch & 0xf) == 0xe) latch |= 1;

	return latch;
}

static READ16_HANDLER( sound_status_msb_r )
{
	int latch = soundlatch3_r(space,0);

	if ((latch & 0xf) == 0xe) latch |= 1;

	return latch<<8;
}

static WRITE16_HANDLER( irq_ack_w )
{
	K056832_b_word_w(space, offset, data, mem_mask);

	if (offset == 3 && ACCESSING_BITS_0_7)
	{
		mw_irq_control = data&0xff;

//      if ((data &0xf0) != 0xd0) logerror("Unknown write to IRQ reg: %x\n", data);

	}
}

/* the interface with the 053247 is weird. The chip can address only 0x1000 bytes */
/* of RAM, but they put 0x10000 there. The CPU can access them all. */
static READ16_HANDLER( K053247_scattered_word_r )
{
	if (offset & 0x0078)
		return space->machine->generic.spriteram.u16[offset];
	else
	{
		offset = (offset & 0x0007) | ((offset & 0x7f80) >> 4);
		return K053247_word_r(space,offset,mem_mask);
	}
}

static WRITE16_HANDLER( K053247_scattered_word_w )
{
	if (offset & 0x0078)
	{
//      mame_printf_debug("spr write %x to %x (PC=%x)\n", data, offset, cpu_get_pc(space->cpu));
		COMBINE_DATA(space->machine->generic.spriteram.u16+offset);
	}
	else
	{
		offset = (offset & 0x0007) | ((offset & 0x7f80) >> 4);

		K053247_word_w(space,offset,data,mem_mask);
	}
}

/* 68000 memory handlers */
/* Mystic Warriors */
static ADDRESS_MAP_START( mystwarr_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM	// main program
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_BASE(&gx_workram)
	AM_RANGE(0x400000, 0x40ffff) AM_READWRITE(K053247_scattered_word_r,K053247_scattered_word_w) AM_BASE_GENERIC(spriteram)
	AM_RANGE(0x480000, 0x4800ff) AM_WRITE(K055555_word_w)
	AM_RANGE(0x482000, 0x48200f) AM_READ(K055673_rom_word_r)
	AM_RANGE(0x482010, 0x48201f) AM_WRITE(K053247_reg_word_w)
	AM_RANGE(0x484000, 0x484007) AM_WRITE(K053246_word_w)
	AM_RANGE(0x48a000, 0x48a01f) AM_WRITE(K054338_word_w)
	AM_RANGE(0x48c000, 0x48c03f) AM_WRITE(K056832_word_w)
	AM_RANGE(0x490000, 0x490001) AM_WRITE(mweeprom_w)
	AM_RANGE(0x492000, 0x492001) AM_WRITENOP	// watchdog
	AM_RANGE(0x494000, 0x494001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x494002, 0x494003) AM_READ_PORT("P3_P4")
	AM_RANGE(0x496000, 0x496001) AM_READ_PORT("IN0")
	AM_RANGE(0x496002, 0x496003) AM_READ(eeprom_r)
	AM_RANGE(0x49800c, 0x49800d) AM_WRITE(sound_cmd1_w)
	AM_RANGE(0x49800e, 0x49800f) AM_WRITE(sound_cmd2_w)
	AM_RANGE(0x498014, 0x498015) AM_READ(sound_status_r)
	AM_RANGE(0x498000, 0x49801f) AM_RAM
	AM_RANGE(0x49a000, 0x49a001) AM_WRITE(sound_irq_w)
	AM_RANGE(0x49c000, 0x49c01f) AM_WRITE(K053252_word_w)
	AM_RANGE(0x49e000, 0x49e007) AM_WRITE(irq_ack_w)	// VSCCS (custom)
	AM_RANGE(0x600000, 0x601fff) AM_READWRITE(K056832_ram_word_r,K056832_ram_word_w)
	AM_RANGE(0x602000, 0x603fff) AM_READWRITE(K056832_ram_word_r,K056832_ram_word_w)	// tilemap RAM mirror read(essential)
	AM_RANGE(0x680000, 0x683fff) AM_READ(K056832_mw_rom_word_r)
	AM_RANGE(0x700000, 0x701fff) AM_RAM_WRITE(paletteram16_xrgb_word_be_w) AM_BASE_GENERIC(paletteram)
#if MW_DEBUG
	AM_RANGE(0x480000, 0x4800ff) AM_READ(K055555_word_r)
	AM_RANGE(0x482010, 0x48201f) AM_READ(K053247_reg_word_r)
	AM_RANGE(0x484000, 0x484007) AM_READ(K053246_reg_word_r)
	AM_RANGE(0x48a000, 0x48a01f) AM_READ(K054338_word_r)
	AM_RANGE(0x48c000, 0x48c03f) AM_READ(K056832_word_r)
	AM_RANGE(0x49c000, 0x49c01f) AM_READ(K053252_word_r)
#endif
ADDRESS_MAP_END

/* Metamorphic Force */
static ADDRESS_MAP_START( metamrph_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM	// main program
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_BASE(&gx_workram)
	AM_RANGE(0x210000, 0x210fff) AM_READWRITE(K053247_word_r,K053247_word_w)
	AM_RANGE(0x211000, 0x21ffff) AM_RAM
	AM_RANGE(0x240000, 0x240007) AM_WRITE(K053246_word_w)
	AM_RANGE(0x244000, 0x24400f) AM_READ(K055673_rom_word_r)
	AM_RANGE(0x244010, 0x24401f) AM_WRITE(K053247_reg_word_w)
	AM_RANGE(0x24c000, 0x24ffff) AM_READWRITE(K053250_0_ram_r,K053250_0_ram_w) // "LVC RAM" (53250_ram)
	AM_RANGE(0x250000, 0x25000f) AM_READWRITE(K053250_0_r,K053250_0_w)
	AM_RANGE(0x254000, 0x25401f) AM_WRITE(K054338_word_w)
	AM_RANGE(0x258000, 0x2580ff) AM_WRITE(K055555_word_w)
	AM_RANGE(0x260000, 0x26001f) AM_WRITE(K053252_word_w)
	AM_RANGE(0x264000, 0x264001) AM_WRITE(sound_irq_w)
	AM_RANGE(0x26800c, 0x26800d) AM_WRITE(sound_cmd1_w)
	AM_RANGE(0x26800e, 0x26800f) AM_WRITE(sound_cmd2_w)
	AM_RANGE(0x268014, 0x268015) AM_READ(sound_status_r)
	AM_RANGE(0x268000, 0x26801f) AM_RAM
	AM_RANGE(0x26c000, 0x26c007) AM_WRITE(K056832_b_word_w)
	AM_RANGE(0x270000, 0x27003f) AM_WRITE(K056832_word_w)
	AM_RANGE(0x274000, 0x274001) AM_READ_PORT("P1_P3")
	AM_RANGE(0x274002, 0x274003) AM_READ_PORT("P2_P4")
	AM_RANGE(0x278000, 0x278001) AM_READ_PORT("IN0")
	AM_RANGE(0x278002, 0x278003) AM_READ(eeprom_r)
	AM_RANGE(0x27c000, 0x27c001) AM_READNOP	// watchdog lives here
	AM_RANGE(0x27c000, 0x27c001) AM_WRITE(mmeeprom_w)
	AM_RANGE(0x300000, 0x301fff) AM_READWRITE(K056832_ram_word_r,K056832_ram_word_w)
	AM_RANGE(0x302000, 0x303fff) AM_READWRITE(K056832_ram_word_r,K056832_ram_word_w)	// tilemap RAM mirror read/write (essential)
	AM_RANGE(0x310000, 0x311fff) AM_READ(K056832_mw_rom_word_r)
	AM_RANGE(0x320000, 0x321fff) AM_READ(K053250_0_rom_r)
	AM_RANGE(0x330000, 0x331fff) AM_RAM_WRITE(paletteram16_xrgb_word_be_w) AM_BASE_GENERIC(paletteram)
#if MW_DEBUG
	AM_RANGE(0x240000, 0x240007) AM_READ(K053246_reg_word_r)
	AM_RANGE(0x244010, 0x24401f) AM_READ(K053247_reg_word_r)
	AM_RANGE(0x254000, 0x25401f) AM_READ(K054338_word_r)
	AM_RANGE(0x258000, 0x2580ff) AM_READ(K055555_word_r)
	AM_RANGE(0x260000, 0x26001f) AM_READ(K053252_word_r)
	AM_RANGE(0x26C000, 0x26C007) AM_READ(K056832_b_word_r)
	AM_RANGE(0x270000, 0x27003f) AM_READ(K056832_word_r)
#endif
ADDRESS_MAP_END

/* Violent Storm */
static ADDRESS_MAP_START( viostorm_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM		// main program
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_BASE(&gx_workram)
	AM_RANGE(0x210000, 0x210fff) AM_READWRITE(K053247_word_r,K053247_word_w)
	AM_RANGE(0x211000, 0x21ffff) AM_RAM
	AM_RANGE(0x240000, 0x240007) AM_WRITE(K053246_word_w)
	AM_RANGE(0x244000, 0x24400f) AM_READ(K055673_rom_word_r)
	AM_RANGE(0x244010, 0x24401f) AM_WRITE(K053247_reg_word_w)
	AM_RANGE(0x24c000, 0x24ffff) AM_RAM		// K053250_0_ram_r / K053250_0_ram_w
	AM_RANGE(0x250000, 0x25000f) AM_RAM		// K053250_0_r / K053250_0_w
	AM_RANGE(0x254000, 0x25401f) AM_WRITE(K054338_word_w)
	AM_RANGE(0x258000, 0x2580ff) AM_WRITE(K055555_word_w)
	AM_RANGE(0x25c000, 0x25c03f) AM_READWRITE(K055550_word_r,K055550_word_w)
	AM_RANGE(0x260000, 0x26001f) AM_WRITE(K053252_word_w)
	AM_RANGE(0x264000, 0x264001) AM_WRITE(sound_irq_w)
	AM_RANGE(0x26800c, 0x26800d) AM_WRITE(sound_cmd1_w)
	AM_RANGE(0x26800e, 0x26800f) AM_WRITE(sound_cmd2_w)
	AM_RANGE(0x268014, 0x268015) AM_READ(sound_status_r)
	AM_RANGE(0x268000, 0x26801f) AM_RAM
	AM_RANGE(0x26c000, 0x26c007) AM_WRITE(K056832_b_word_w)
	AM_RANGE(0x270000, 0x27003f) AM_WRITE(K056832_word_w)
	AM_RANGE(0x274000, 0x274001) AM_READ_PORT("P1_P3")
	AM_RANGE(0x274002, 0x274003) AM_READ_PORT("P2_P4")
	AM_RANGE(0x278000, 0x278001) AM_READ_PORT("IN0")
	AM_RANGE(0x278002, 0x278003) AM_READ(eeprom_r)
	AM_RANGE(0x27c000, 0x27c001) AM_READNOP		// watchdog lives here
	AM_RANGE(0x27c000, 0x27c001) AM_WRITE(mmeeprom_w)
	AM_RANGE(0x300000, 0x301fff) AM_READWRITE(K056832_ram_word_r,K056832_ram_word_w)
	AM_RANGE(0x302000, 0x303fff) AM_READWRITE(K056832_ram_word_r,K056832_ram_word_w) // tilemap RAM mirror read(essential)
	AM_RANGE(0x304000, 0x3041ff) AM_RAM
	AM_RANGE(0x310000, 0x311fff) AM_READ(K056832_mw_rom_word_r)
	AM_RANGE(0x330000, 0x331fff) AM_RAM_WRITE(paletteram16_xrgb_word_be_w) AM_BASE_GENERIC(paletteram)
#if MW_DEBUG
	AM_RANGE(0x240000, 0x240007) AM_READ(K053246_reg_word_r)
	AM_RANGE(0x244010, 0x24401f) AM_READ(K053247_reg_word_r)
	AM_RANGE(0x254000, 0x25401f) AM_READ(K054338_word_r)
	AM_RANGE(0x258000, 0x2580ff) AM_READ(K055555_word_r)
	AM_RANGE(0x260000, 0x26001f) AM_READ(K053252_word_r)
	AM_RANGE(0x26C000, 0x26C007) AM_READ(K056832_b_word_r)
	AM_RANGE(0x270000, 0x27003f) AM_READ(K056832_word_r)
#endif
ADDRESS_MAP_END

// Martial Champion specific interfaces
static READ16_HANDLER( K053247_martchmp_word_r )
{
	if (offset & 0x0018)
		return space->machine->generic.spriteram.u16[offset];
	else
	{
		offset = (offset & 0x0007) | ((offset & 0x1fe0) >> 2);
		return K053247_word_r(space,offset,mem_mask);
	}
}

static WRITE16_HANDLER( K053247_martchmp_word_w )
{
	if (offset & 0x0018)
	{
		COMBINE_DATA(space->machine->generic.spriteram.u16+offset);
	}
	else
	{
		offset = (offset & 0x0007) | ((offset & 0x1fe0) >> 2);

		K053247_word_w(space,offset,data,mem_mask);
	}
}

static READ16_HANDLER( mccontrol_r )
{
	return mw_irq_control<<8;
}

static WRITE16_HANDLER( mccontrol_w )
{
	if (ACCESSING_BITS_8_15)
	{
		mw_irq_control = data>>8;
		// bit 0 = watchdog
		// bit 1 = IRQ enable
		// bit 2 = OBJCHA

		K053246_set_OBJCHA_line((data&0x04) ? ASSERT_LINE : CLEAR_LINE);

//      if (data & 0xf8) logerror("Unk write %x to mccontrol\n", data);

	}

//  else logerror("write %x to LSB of mccontrol\n", data);

}

/* Martial Champion */
static ADDRESS_MAP_START( martchmp_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM									// main program
	AM_RANGE(0x100000, 0x10ffff) AM_RAM AM_BASE(&gx_workram)			// work RAM
	AM_RANGE(0x300000, 0x3fffff) AM_ROM									// data ROM
	AM_RANGE(0x400000, 0x4000ff) AM_WRITE(K055555_word_w)				// PCU2
	AM_RANGE(0x402000, 0x40200f) AM_READ(K055673_rom_word_r)			// sprite ROM readback
	AM_RANGE(0x402010, 0x40201f) AM_WRITE(K053247_reg_word_w)			// OBJSET2
	AM_RANGE(0x404000, 0x404007) AM_WRITE(K053246_word_w)				// OBJSET1
	AM_RANGE(0x40a000, 0x40a01f) AM_WRITE(K054338_word_w)				// CLTC
	AM_RANGE(0x40c000, 0x40c03f) AM_WRITE(K056832_word_w)				// VACSET
	AM_RANGE(0x40e000, 0x40e03f) AM_WRITE(K053990_martchmp_word_w)		// protection
	AM_RANGE(0x410000, 0x410001) AM_WRITE(mweeprom_w)
	AM_RANGE(0x412000, 0x412001) AM_READWRITE(mccontrol_r,mccontrol_w)
	AM_RANGE(0x414000, 0x414001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x414002, 0x414003) AM_READ_PORT("P3_P4")
	AM_RANGE(0x416000, 0x416001) AM_READ_PORT("IN0")
	AM_RANGE(0x416002, 0x416003) AM_READ(eeprom_r)					// eeprom read
	AM_RANGE(0x418014, 0x418015) AM_READ(sound_status_r)				// z80 status
	AM_RANGE(0x41800c, 0x41800d) AM_WRITE(sound_cmd1_w)
	AM_RANGE(0x41800e, 0x41800f) AM_WRITE(sound_cmd2_w)
	AM_RANGE(0x418000, 0x41801f) AM_RAM									// sound regs fall through
	AM_RANGE(0x41a000, 0x41a001) AM_WRITE(sound_irq_w)
	AM_RANGE(0x41c000, 0x41c01f) AM_WRITE(K053252_word_w)				// CCU
	AM_RANGE(0x41e000, 0x41e007) AM_WRITE(K056832_b_word_w)				// VSCCS
	AM_RANGE(0x480000, 0x483fff) AM_READWRITE(K053247_martchmp_word_r,K053247_martchmp_word_w) AM_BASE_GENERIC(spriteram)	// sprite RAM
	AM_RANGE(0x600000, 0x601fff) AM_RAM_WRITE(paletteram16_xrgb_word_be_w) AM_BASE_GENERIC(paletteram)						// palette RAM
	AM_RANGE(0x680000, 0x681fff) AM_READWRITE(K056832_ram_word_r,K056832_ram_word_w)	// tilemap RAM
	AM_RANGE(0x682000, 0x683fff) AM_READWRITE(K056832_ram_word_r,K056832_ram_word_w)	// tilemap RAM mirror read/write (essential)
	AM_RANGE(0x700000, 0x703fff) AM_READ(K056832_mw_rom_word_r)			// tile ROM readback
#if MW_DEBUG
	AM_RANGE(0x400000, 0x4000ff) AM_READ(K055555_word_r)
	AM_RANGE(0x402010, 0x40201f) AM_READ(K053247_reg_word_r)
	AM_RANGE(0x404000, 0x404007) AM_READ(K053246_reg_word_r)
	AM_RANGE(0x40a000, 0x40a01f) AM_READ(K054338_word_r)
	AM_RANGE(0x40c000, 0x40c03f) AM_READ(K056832_word_r)
	AM_RANGE(0x41c000, 0x41c01f) AM_READ(K053252_word_r)
	AM_RANGE(0x41e000, 0x41e007) AM_READ(K056832_b_word_r)
#endif
ADDRESS_MAP_END

/* Ultimate Battler Dadandarn */
static ADDRESS_MAP_START( dadandrn_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM							// main program and data ROM
	AM_RANGE(0x400000, 0x40ffff) AM_READWRITE(K053247_scattered_word_r,K053247_scattered_word_w) AM_BASE_GENERIC(spriteram)
	AM_RANGE(0x410000, 0x411fff) AM_READWRITE(K056832_ram_word_r,K056832_ram_word_w)	// tilemap RAM
	AM_RANGE(0x412000, 0x413fff) AM_READWRITE(K056832_ram_word_r,K056832_ram_word_w)	// tilemap RAM mirror read/write (essential)
	AM_RANGE(0x420000, 0x421fff) AM_RAM_WRITE(paletteram16_xrgb_word_be_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x430000, 0x430007) AM_WRITE(K053246_word_w)
	AM_RANGE(0x440000, 0x443fff) AM_READ(K056832_mw_rom_word_r)
	AM_RANGE(0x450000, 0x45000f) AM_READ(K055673_rom_word_r)
	AM_RANGE(0x450010, 0x45001f) AM_WRITE(K053247_reg_word_w)
	AM_RANGE(0x460000, 0x46001f) AM_WRITEONLY AM_BASE(&K053936_0_ctrl)
	AM_RANGE(0x470000, 0x470fff) AM_RAM AM_BASE(&K053936_0_linectrl)
	AM_RANGE(0x480000, 0x48003f) AM_WRITE(K056832_word_w)		// VACSET
	AM_RANGE(0x482000, 0x482007) AM_WRITE(K056832_b_word_w)	// VSCCS
	AM_RANGE(0x484000, 0x484003) AM_WRITE(ddd_053936_clip_w)
	AM_RANGE(0x486000, 0x48601f) AM_WRITE(K053252_word_w)
	AM_RANGE(0x488000, 0x4880ff) AM_WRITE(K055555_word_w)
	AM_RANGE(0x48a00c, 0x48a00d) AM_WRITE(sound_cmd1_msb_w)
	AM_RANGE(0x48a00e, 0x48a00f) AM_WRITE(sound_cmd2_msb_w)
	AM_RANGE(0x48a014, 0x48a015) AM_READ(sound_status_msb_r)
	AM_RANGE(0x48a000, 0x48a01f) AM_RAM					// sound regs fall-through
	AM_RANGE(0x48c000, 0x48c01f) AM_WRITE(K054338_word_w)
	AM_RANGE(0x48e000, 0x48e001) AM_READ_PORT("IN0_P1")	// bit 3 (0x8) is test switch
	AM_RANGE(0x48e020, 0x48e021) AM_READ(dddeeprom_r)
	AM_RANGE(0x600000, 0x60ffff) AM_RAM AM_BASE(&gx_workram)
	AM_RANGE(0x680000, 0x68003f) AM_READWRITE(K055550_word_r,K055550_word_w)
	AM_RANGE(0x6a0000, 0x6a0001) AM_WRITE(mmeeprom_w)
	AM_RANGE(0x6c0000, 0x6c0001) AM_WRITE(ddd_053936_enable_w)
	AM_RANGE(0x6e0000, 0x6e0001) AM_WRITE(sound_irq_w)
	AM_RANGE(0x800000, 0x87ffff) AM_READ(ddd_053936_tilerom_0_r)	// 256k tilemap readback
	AM_RANGE(0xa00000, 0xa7ffff) AM_READ(ddd_053936_tilerom_1_r) // 128k tilemap readback
	AM_RANGE(0xc00000, 0xdfffff) AM_READ(ddd_053936_tilerom_2_r)	// tile character readback
	AM_RANGE(0xe00000, 0xe00001) AM_WRITENOP	// watchdog
#if MW_DEBUG
	AM_RANGE(0x430000, 0x430007) AM_READ(K053246_reg_word_r)
	AM_RANGE(0x450010, 0x45001f) AM_READ(K053247_reg_word_r)
	AM_RANGE(0x480000, 0x48003f) AM_READ(K056832_word_r)
	AM_RANGE(0x482000, 0x482007) AM_READ(K056832_b_word_r)
	AM_RANGE(0x486000, 0x48601f) AM_READ(K053252_word_r)
	AM_RANGE(0x488000, 0x4880ff) AM_READ(K055555_word_r)
	AM_RANGE(0x48c000, 0x48c01f) AM_READ(K054338_word_r)
#endif
ADDRESS_MAP_END

/* Gaiapolis */
// a00000 = the 128k tilemap
// 800000 = the 256k tilemap
// c00000 = 936 tiles (7fffff window)
static ADDRESS_MAP_START( gaiapols_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x2fffff) AM_ROM								// main program
	AM_RANGE(0x400000, 0x40ffff) AM_READWRITE(K053247_scattered_word_r,K053247_scattered_word_w) AM_BASE_GENERIC(spriteram)
	AM_RANGE(0x410000, 0x411fff) AM_READWRITE(K056832_ram_word_r,K056832_ram_word_w)		// tilemap RAM
	AM_RANGE(0x412000, 0x413fff) AM_READWRITE(K056832_ram_word_r,K056832_ram_word_w)		// tilemap RAM mirror read / write (essential)
	AM_RANGE(0x420000, 0x421fff) AM_RAM_WRITE(paletteram16_xrgb_word_be_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x430000, 0x430007) AM_WRITE(K053246_word_w)
	AM_RANGE(0x440000, 0x441fff) AM_READ(K056832_mw_rom_word_r)
	AM_RANGE(0x450000, 0x45000f) AM_READ(K055673_rom_word_r)
	AM_RANGE(0x450010, 0x45001f) AM_WRITE(K053247_reg_word_w)
	AM_RANGE(0x460000, 0x46001f) AM_WRITEONLY AM_BASE(&K053936_0_ctrl)
	AM_RANGE(0x470000, 0x470fff) AM_RAM AM_BASE(&K053936_0_linectrl)
	AM_RANGE(0x480000, 0x48003f) AM_WRITE(K056832_word_w)			// VACSET
	AM_RANGE(0x482000, 0x482007) AM_WRITE(K056832_b_word_w)			// VSCCS
	AM_RANGE(0x484000, 0x484003) AM_WRITE(ddd_053936_clip_w)
	AM_RANGE(0x486000, 0x48601f) AM_WRITE(K053252_word_w)
	AM_RANGE(0x488000, 0x4880ff) AM_WRITE(K055555_word_w)
	AM_RANGE(0x48a00c, 0x48a00d) AM_WRITE(sound_cmd1_msb_w)
	AM_RANGE(0x48a00e, 0x48a00f) AM_WRITE(sound_cmd2_msb_w)
	AM_RANGE(0x48a014, 0x48a015) AM_READ(sound_status_msb_r)
	AM_RANGE(0x48a000, 0x48a01f) AM_RAM								// sound regs fall-through
	AM_RANGE(0x48c000, 0x48c01f) AM_WRITE(K054338_word_w)
	AM_RANGE(0x48e000, 0x48e001) AM_READ_PORT("IN0_P1")				// bit 3 (0x8) is test switch
	AM_RANGE(0x48e020, 0x48e021) AM_READ(dddeeprom_r)
	AM_RANGE(0x600000, 0x60ffff) AM_RAM AM_BASE(&gx_workram)
	AM_RANGE(0x660000, 0x6600ff) AM_READWRITE(K054000_lsb_r,K054000_lsb_w)
	AM_RANGE(0x6a0000, 0x6a0001) AM_WRITE(mmeeprom_w)
	AM_RANGE(0x6c0000, 0x6c0001) AM_WRITE(ddd_053936_enable_w)
	AM_RANGE(0x6e0000, 0x6e0001) AM_WRITE(sound_irq_w)
	AM_RANGE(0x800000, 0x87ffff) AM_READ(gai_053936_tilerom_0_r)	// 256k tilemap readback
	AM_RANGE(0xa00000, 0xa7ffff) AM_READ(ddd_053936_tilerom_1_r)	// 128k tilemap readback
	AM_RANGE(0xc00000, 0xdfffff) AM_READ(gai_053936_tilerom_2_r)	// tile character readback
	AM_RANGE(0xe00000, 0xe00001) AM_WRITENOP	// watchdog
#if MW_DEBUG
	AM_RANGE(0x430000, 0x430007) AM_READ(K053246_reg_word_r)
	AM_RANGE(0x450010, 0x45001f) AM_READ(K053247_reg_word_r)
	AM_RANGE(0x480000, 0x48003f) AM_READ(K056832_word_r)
	AM_RANGE(0x482000, 0x482007) AM_READ(K056832_b_word_r)
	AM_RANGE(0x486000, 0x48601f) AM_READ(K053252_word_r)
	AM_RANGE(0x488000, 0x4880ff) AM_READ(K055555_word_r)
	AM_RANGE(0x48c000, 0x48c01f) AM_READ(K054338_word_r)
#endif
ADDRESS_MAP_END

/**********************************************************************************/

static int cur_sound_region;

static void reset_sound_region(running_machine *machine)
{
	memory_set_bankptr(machine, "bank2", memory_region(machine, "soundcpu") + 0x10000 + cur_sound_region*0x4000);
}

static WRITE8_HANDLER( sound_bankswitch_w )
{
	cur_sound_region = (data & 0xf);
	reset_sound_region(space->machine);
}

/* sound memory maps

   there are 2 sound boards: the martial champion single-'539 version
   and the dual-'539 version used by run and gun, violent storm, monster maulers,
   gaiapolous, metamorphic force, and mystic warriors.  Their memory maps are
   quite similar to xexex/gijoe/asterix's sound.
 */

static ADDRESS_MAP_START( mystwarr_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2")
	AM_RANGE(0x0000, 0xbfff) AM_WRITENOP
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe22f) AM_DEVREADWRITE("konami1", k054539_r,k054539_w)
	AM_RANGE(0xe230, 0xe3ff) AM_RAM
	AM_RANGE(0xe400, 0xe62f) AM_DEVREADWRITE("konami2", k054539_r,k054539_w)
	AM_RANGE(0xe630, 0xe7ff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_WRITE(soundlatch3_w)
	AM_RANGE(0xf002, 0xf002) AM_READ(soundlatch_r)
	AM_RANGE(0xf003, 0xf003) AM_READ(soundlatch2_r)
	AM_RANGE(0xf800, 0xf800) AM_WRITE(sound_bankswitch_w)
	AM_RANGE(0xfff0, 0xfff3) AM_WRITENOP	// unknown write
ADDRESS_MAP_END


static const k054539_interface k054539_config =
{
	"shared"
};

/**********************************************************************************/

static INPUT_PORTS_START( mystwarr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )	/* EEPROM ready (always 1) */
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* game loops if this is set */
	PORT_DIPNAME( 0x10, 0x00, "Sound Output" )
	PORT_DIPSETTING(    0x10, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Stereo ) )
	PORT_DIPNAME( 0x20, 0x20, "Coin Mechanism" )
	PORT_DIPSETTING(    0x20, "Common" )
	PORT_DIPSETTING(    0x00, "Independent" )
	PORT_DIPNAME( 0x40, 0x40, "Number of Players" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_P2")
	KONAMI16_LSB(1, IPT_BUTTON3, IPT_START1 )
	KONAMI16_MSB(2, IPT_BUTTON3, IPT_START2 )

	PORT_START("P3_P4")
	KONAMI16_LSB(3, IPT_BUTTON3, IPT_START3 )
	KONAMI16_MSB(4, IPT_BUTTON3, IPT_START4 )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
INPUT_PORTS_END

static INPUT_PORTS_START( metamrph )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )	/* EEPROM ready (always 1) */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x00, "Sound Output" )
	PORT_DIPSETTING(    0x10, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Stereo ) )
	PORT_DIPNAME( 0x20, 0x20, "Coin Mechanism" )
	PORT_DIPSETTING(    0x20, "Common" )
	PORT_DIPSETTING(    0x00, "Independent" )
	PORT_DIPNAME( 0x40, 0x40, "Number of Players" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPNAME( 0x80, 0x80, "Continuous Energy Increment" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("P1_P3")
	KONAMI16_LSB(1, IPT_BUTTON3, IPT_START1 )
	KONAMI16_MSB(3, IPT_BUTTON3, IPT_START3 )

	PORT_START("P2_P4")
	KONAMI16_LSB(2, IPT_BUTTON3, IPT_START2 )
	KONAMI16_MSB(4, IPT_BUTTON3, IPT_START4 )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
INPUT_PORTS_END

static INPUT_PORTS_START( viostorm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )	/* EEPROM ready (always 1) */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x00, "Sound Output" )
	PORT_DIPSETTING(    0x10, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Stereo ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Coin Mechanism" )
	PORT_DIPSETTING(    0x40, "Common" )
	PORT_DIPSETTING(    0x00, "Independent" )
	PORT_DIPNAME( 0x80, 0x80, "Number of Players" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("P1_P3")
	KONAMI16_LSB(1, IPT_BUTTON3, IPT_START1 )
	KONAMI16_MSB(3, IPT_BUTTON3, IPT_START3 )

	PORT_START("P2_P4")
	KONAMI16_LSB(2, IPT_BUTTON3, IPT_START2 )
	KONAMI16_MSB(4, IPT_BUTTON3, IPT_START4 )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
INPUT_PORTS_END

static INPUT_PORTS_START( dadandrn )
	PORT_START("IN0_P1")
	KONAMI8_B123_START(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )	/* EEPROM ready (always 1) */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x00, "Sound Output" )
	PORT_DIPSETTING(    0x10, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Stereo ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	KONAMI8_B123_START(2)

	PORT_START("P3")
	KONAMI8_B123_START(3)

	PORT_START("P4")
	KONAMI8_B123_START(4)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
INPUT_PORTS_END

static INPUT_PORTS_START( martchmp )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )	/* EEPROM ready (always 1) */
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* game loops if this is set */
	PORT_DIPNAME( 0x10, 0x00, "Sound Output" )
	PORT_DIPSETTING(    0x10, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Stereo ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_P2")
	KONAMI16_LSB(1, IPT_BUTTON3, IPT_START1 )
	KONAMI16_MSB(2, IPT_BUTTON3, IPT_START2 )

	PORT_START("P3_P4")
	KONAMI16_LSB(3, IPT_BUTTON3, IPT_START3 )
	KONAMI16_MSB(4, IPT_BUTTON3, IPT_START4 )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
INPUT_PORTS_END

/**********************************************************************************/

static const gfx_layout bglayout_4bpp =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4, 8*4, 9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*64
};

static const gfx_layout bglayout_8bpp =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	16*128
};

static GFXDECODE_START( gaiapols )
	GFXDECODE_ENTRY( "gfx3", 0, bglayout_4bpp, 0x0000, 128 )
GFXDECODE_END

static GFXDECODE_START( dadandrn )
	GFXDECODE_ENTRY( "gfx3", 0, bglayout_8bpp, 0x0000, 8 )
GFXDECODE_END

static STATE_POSTLOAD( mystwarr_postload )
{
	reset_sound_region(machine);
}

static MACHINE_START( mystwarr )
{
	/* set default bankswitch */
	cur_sound_region = 2;
	reset_sound_region(machine);

	mw_irq_control = 0;

	state_save_register_global(machine, mw_irq_control);
	state_save_register_global(machine, cur_sound_region);
	state_save_register_postload(machine, mystwarr_postload, NULL);
}

static MACHINE_RESET(mystwarr)
{
	running_device *k054539_1 = machine->device("konami1");
	running_device *k054539_2 = machine->device("konami2");
	int i;

	// soften chorus(chip 0 channel 0-3), boost voice(chip 0 channel 4-7)
	for (i=0; i<=3; i++)
	{
		k054539_set_gain(k054539_1, i, 0.8);
		k054539_set_gain(k054539_1, i+4, 2.0);
	}

	// soften percussions(chip 1 channel 0-7)
	for (i=0; i<=7; i++) k054539_set_gain(k054539_2, i, 0.5);
}

static MACHINE_RESET(dadandrn)
{
	running_device *k054539_1 = machine->device("konami1");
	int i;

	// boost voice(chip 0 channel 4-7)
	for (i=4; i<=7; i++) k054539_set_gain(k054539_1, i, 2.0);
}

static MACHINE_RESET(viostorm)
{
	running_device *k054539_1 = machine->device("konami1");
	int i;

	// boost voice(chip 0 channel 4-7)
	for (i=4; i<=7; i++) k054539_set_gain(k054539_1, i, 2.0);
}

static MACHINE_RESET(metamrph)
{
	running_device *k054539_1 = machine->device("konami1");
	running_device *k054539_2 = machine->device("konami2");
	int i;

	// boost voice(chip 0 channel 4-7) and soften other channels
	for (i=0; i<=3; i++)
	{
		k054539_set_gain(k054539_1, i,   0.8);
		k054539_set_gain(k054539_1, i+4, 1.8);
		k054539_set_gain(k054539_2, i,   0.8);
		k054539_set_gain(k054539_2, i+4, 0.8);
	}
}

static MACHINE_RESET(martchmp)
{
	running_device *k054539_1 = machine->device("konami1");
	int i;

	k054539_init_flags(k054539_1, K054539_REVERSE_STEREO);

	// boost voice(chip 0 channel 4-7)
	for (i=4; i<=7; i++) k054539_set_gain(k054539_1, i, 1.4);
}

static MACHINE_RESET(gaiapols)
{
	running_device *k054539_1 = machine->device("konami1");
	int i;

	// boost voice(chip 0 channel 5-7)
	for (i=5; i<=7; i++) k054539_set_gain(k054539_1, i, 2.0);
}

static MACHINE_DRIVER_START( mystwarr )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz (confirmed) */
	MDRV_CPU_PROGRAM_MAP(mystwarr_map)
	MDRV_CPU_VBLANK_INT_HACK(mystwarr_interrupt, 3)

	MDRV_CPU_ADD("soundcpu", Z80, 8000000)
	MDRV_CPU_PROGRAM_MAP(mystwarr_sound_map)
	MDRV_CPU_PERIODIC_INT(nmi_line_pulse, 480)

	MDRV_QUANTUM_TIME(HZ(1920))

	MDRV_EEPROM_ADD("eeprom", eeprom_intf)

	MDRV_MACHINE_START(mystwarr)
	MDRV_MACHINE_RESET(mystwarr)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS | VIDEO_UPDATE_AFTER_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
//  MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_RAW_PARAMS(6000000, 288+16+32+48, 0, 287, 224+16+8+16, 0, 223)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(600))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(24, 24+288-1, 16, 16+224-1)

	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(mystwarr)
	MDRV_VIDEO_UPDATE(mystwarr)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("konami1", K054539, 48000)
	MDRV_SOUND_CONFIG(k054539_config)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.0)	/* stereo channels are inverted */
	MDRV_SOUND_ROUTE(1, "lspeaker", 1.0)

	MDRV_SOUND_ADD("konami2", K054539, 48000)
	MDRV_SOUND_CONFIG(k054539_config)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.0)	/* stereo channels are inverted */
	MDRV_SOUND_ROUTE(1, "lspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( viostorm )
	MDRV_IMPORT_FROM(mystwarr)

	MDRV_MACHINE_RESET(viostorm)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(viostorm_map)
	MDRV_CPU_VBLANK_INT_HACK(metamrph_interrupt, 40)

	/* video hardware */
	MDRV_VIDEO_START(viostorm)
	MDRV_VIDEO_UPDATE(metamrph)

	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(900))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(40, 40+384-1, 16, 16+224-1)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( metamrph )
	MDRV_IMPORT_FROM(mystwarr)

	MDRV_MACHINE_RESET(metamrph)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(metamrph_map)
	MDRV_CPU_VBLANK_INT_HACK(metamrph_interrupt, 40)

	/* video hardware */
	MDRV_VIDEO_START(metamrph)
	MDRV_VIDEO_UPDATE(metamrph)

	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(900))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(24, 24+288-1, 17, 17+224-1)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( dadandrn )
	MDRV_IMPORT_FROM(mystwarr)

	MDRV_MACHINE_RESET(dadandrn)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(dadandrn_map)
	MDRV_CPU_VBLANK_INT("screen", ddd_interrupt)

	MDRV_GFXDECODE(dadandrn)

	/* video hardware */
	MDRV_VIDEO_START(dadandrn)
	MDRV_VIDEO_UPDATE(dadandrn)

	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(600))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(24, 24+288-1, 17, 17+224-1)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( gaiapols )
	MDRV_IMPORT_FROM(mystwarr)

	MDRV_MACHINE_RESET(gaiapols)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(gaiapols_map)
	MDRV_CPU_VBLANK_INT("screen", ddd_interrupt)

	MDRV_GFXDECODE(gaiapols)

	MDRV_DEVICE_REMOVE("eeprom")
	MDRV_EEPROM_ADD("eeprom", gaia_eeprom_intf)

	/* video hardware */
	MDRV_VIDEO_START(gaiapols)
	MDRV_VIDEO_UPDATE(dadandrn)

	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_RAW_PARAMS(8000000, 384+24+64+40, 0, 383, 224+16+8+16, 0, 223)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(600))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(40, 40+376-1, 16, 16+224-1)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( martchmp )
	MDRV_IMPORT_FROM(mystwarr)

	MDRV_MACHINE_RESET(martchmp)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(martchmp_map)
	MDRV_CPU_VBLANK_INT_HACK(mchamp_interrupt, 2)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS | VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_VIDEO_START(martchmp)
	MDRV_VIDEO_UPDATE(martchmp)

	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(32, 32+384-1, 16, 16+224-1)
MACHINE_DRIVER_END

/**********************************************************************************/

#define ROM_LOADTILE_WORD(name,offset,length,crc) ROMX_LOAD(name, offset, length, crc, ROM_GROUPWORD | ROM_SKIP(3) | ROM_REVERSE)
#define ROM_LOADTILE_BYTE(name,offset,length,crc) ROMX_LOAD(name, offset, length, crc, ROM_GROUPBYTE | ROM_SKIP(4))

ROM_START( mystwarr )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "128eaa01.20f", 0x000000, 0x40000, CRC(508f249c) SHA1(d27a272ec63e4a39379c4a23fe15c4fb45674cb4) )
	ROM_LOAD16_BYTE( "128eaa02.20g", 0x000001, 0x40000, CRC(f8ffa352) SHA1(678c59d5fbb85d808e842947621b7f13669d35b5) )
	ROM_LOAD16_BYTE( "128a03.19f",   0x100000, 0x80000, CRC(e98094f3) SHA1(a3f9b804ff487f792a00ce85a383868ab0b1b5d8) )
	ROM_LOAD16_BYTE( "128a04.19g",   0x100001, 0x80000, CRC(88c6a3e4) SHA1(7c2361f716a2320730a3dd6723a271e349ad61c3) )

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("128a05.6b", 0x000000, 0x020000, CRC(0e5194e0) SHA1(83356158d561f1b8e21f6ae5936b61da834a0545) )
	ROM_RELOAD(           0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x500000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "128a08.1h", 0x000000, 1*1024*1024, CRC(63d6cfa0) SHA1(324bf25cf79aa030d2dcc94a53c1984eb8abec3a) )
	ROM_LOADTILE_WORD( "128a09.1k", 0x000002, 1*1024*1024, CRC(573a7725) SHA1(f2fef32053ed2a65c6c3ddd3e1657a866aa80b3e) )
	ROM_LOADTILE_BYTE( "128a10.3h", 0x000004, 512*1024, CRC(558e545a) SHA1(cac53e545f3f8980d431443f2c3b8b95e6077d1c) )

	/* sprites */
	ROM_REGION( 0x500000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "128a16.22k", 0x000000, 1*1024*1024, CRC(459b6407) SHA1(e4dace4912f9558bee75a8e95ee2637f5e950b47) )
	ROM_LOAD64_WORD( "128a15.20k", 0x000002, 1*1024*1024, CRC(6bbfedf4) SHA1(0b3acb2b34c722ddc60c0e64e12baa1f225e4fbb) )
	ROM_LOAD64_WORD( "128a14.19k", 0x000004, 1*1024*1024, CRC(f7bd89dd) SHA1(c9b2ebd5a49840f8b260d53c25cfcc238d21c75c) )
	ROM_LOAD64_WORD( "128a13.17k", 0x000006, 1*1024*1024, CRC(e89b66a2) SHA1(fce6e56d1759ffe987766426ecb28e9015a500b7) )
	ROM_LOAD16_BYTE( "128a12.12k", 0x400000, 512*1024, CRC(63de93e2) SHA1(c9a50e7beff1cbbc5d5820664adbd54d52782c54) )
	ROM_LOAD16_BYTE( "128a11.10k", 0x400001, 512*1024, CRC(4eac941a) SHA1(c0a33f4b975ebee217fd335001839992f4c0bdc8) )

	/* road generator */
	ROM_REGION( 0x40000, "gfx3", ROMREGION_ERASE00 )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "128a06.2d", 0x000000, 2*1024*1024, CRC(88ed598c) SHA1(3c123e26b3a12541df77b368bc0e0d486f5622b6) )
	ROM_LOAD( "128a07.1d", 0x200000, 2*1024*1024, CRC(db79a66e) SHA1(b7e118ed26bac557038e8ae6cb77f23f3da5646f) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "mystwarr.nv", 0x0000, 0x080, CRC(28df2269) SHA1(3f071c97662745a199f96964e2e79f795bd5a391) )
ROM_END

ROM_START( mystwarru )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "128uaa01.20f", 0x000000, 0x40000, CRC(3a89aafd) SHA1(6d2ebb7e04d262545276c8dbe1c63405e5de4901) )
	ROM_LOAD16_BYTE( "128uaa02.20g", 0x000001, 0x40000, CRC(de07410f) SHA1(4583cb4402b3b046f185fa6d5a1cfaa8fe0d858c) )
	ROM_LOAD16_BYTE( "128a03.19f",   0x100000, 0x80000, CRC(e98094f3) SHA1(a3f9b804ff487f792a00ce85a383868ab0b1b5d8) )
	ROM_LOAD16_BYTE( "128a04.19g",   0x100001, 0x80000, CRC(88c6a3e4) SHA1(7c2361f716a2320730a3dd6723a271e349ad61c3) )

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("128a05.6b", 0x000000, 0x020000, CRC(0e5194e0) SHA1(83356158d561f1b8e21f6ae5936b61da834a0545) )
	ROM_RELOAD(           0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x500000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "128a08.1h", 0x000000, 1*1024*1024, CRC(63d6cfa0) SHA1(324bf25cf79aa030d2dcc94a53c1984eb8abec3a) )
	ROM_LOADTILE_WORD( "128a09.1k", 0x000002, 1*1024*1024, CRC(573a7725) SHA1(f2fef32053ed2a65c6c3ddd3e1657a866aa80b3e) )
	ROM_LOADTILE_BYTE( "128a10.3h", 0x000004, 512*1024, CRC(558e545a) SHA1(cac53e545f3f8980d431443f2c3b8b95e6077d1c) )

	/* sprites */
	ROM_REGION( 0x500000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "128a16.22k", 0x000000, 1*1024*1024, CRC(459b6407) SHA1(e4dace4912f9558bee75a8e95ee2637f5e950b47) )
	ROM_LOAD64_WORD( "128a15.20k", 0x000002, 1*1024*1024, CRC(6bbfedf4) SHA1(0b3acb2b34c722ddc60c0e64e12baa1f225e4fbb) )
	ROM_LOAD64_WORD( "128a14.19k", 0x000004, 1*1024*1024, CRC(f7bd89dd) SHA1(c9b2ebd5a49840f8b260d53c25cfcc238d21c75c) )
	ROM_LOAD64_WORD( "128a13.17k", 0x000006, 1*1024*1024, CRC(e89b66a2) SHA1(fce6e56d1759ffe987766426ecb28e9015a500b7) )
	ROM_LOAD16_BYTE( "128a12.12k", 0x400000, 512*1024, CRC(63de93e2) SHA1(c9a50e7beff1cbbc5d5820664adbd54d52782c54) )
	ROM_LOAD16_BYTE( "128a11.10k", 0x400001, 512*1024, CRC(4eac941a) SHA1(c0a33f4b975ebee217fd335001839992f4c0bdc8) )

	/* road generator */
	ROM_REGION( 0x40000, "gfx3", ROMREGION_ERASE00 )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "128a06.2d", 0x000000, 2*1024*1024, CRC(88ed598c) SHA1(3c123e26b3a12541df77b368bc0e0d486f5622b6) )
	ROM_LOAD( "128a07.1d", 0x200000, 2*1024*1024, CRC(db79a66e) SHA1(b7e118ed26bac557038e8ae6cb77f23f3da5646f) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "mystwarru.nv", 0x0000, 0x080, CRC(1a2597c7) SHA1(3d85817fe42776c862a5930b8ad131443bc0172e) )
ROM_END

ROM_START( mystwarrj )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "128jaa01.20f", 0x000000, 0x40000, CRC(49c37bfe) SHA1(177843899055476c9d2716ba494ac3892993eee5) )
	ROM_LOAD16_BYTE( "128jaa02.20g", 0x000001, 0x40000, CRC(e39fb3bb) SHA1(0467b51f66c32ffa8fae3b00d43c6d4aa19b24ef) )
	ROM_LOAD16_BYTE( "128a03.19f",   0x100000, 0x80000, CRC(e98094f3) SHA1(a3f9b804ff487f792a00ce85a383868ab0b1b5d8) )
	ROM_LOAD16_BYTE( "128a04.19g",   0x100001, 0x80000, CRC(88c6a3e4) SHA1(7c2361f716a2320730a3dd6723a271e349ad61c3) )

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("128a05.6b", 0x000000, 0x020000, CRC(0e5194e0) SHA1(83356158d561f1b8e21f6ae5936b61da834a0545) )
	ROM_RELOAD(           0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x500000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "128a08.1h", 0x000000, 1*1024*1024, CRC(63d6cfa0) SHA1(324bf25cf79aa030d2dcc94a53c1984eb8abec3a) )
	ROM_LOADTILE_WORD( "128a09.1k", 0x000002, 1*1024*1024, CRC(573a7725) SHA1(f2fef32053ed2a65c6c3ddd3e1657a866aa80b3e) )
	ROM_LOADTILE_BYTE( "128a10.3h", 0x000004, 512*1024, CRC(558e545a) SHA1(cac53e545f3f8980d431443f2c3b8b95e6077d1c) )

	/* sprites */
	ROM_REGION( 0x500000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "128a16.22k", 0x000000, 1*1024*1024, CRC(459b6407) SHA1(e4dace4912f9558bee75a8e95ee2637f5e950b47) )
	ROM_LOAD64_WORD( "128a15.20k", 0x000002, 1*1024*1024, CRC(6bbfedf4) SHA1(0b3acb2b34c722ddc60c0e64e12baa1f225e4fbb) )
	ROM_LOAD64_WORD( "128a14.19k", 0x000004, 1*1024*1024, CRC(f7bd89dd) SHA1(c9b2ebd5a49840f8b260d53c25cfcc238d21c75c) )
	ROM_LOAD64_WORD( "128a13.17k", 0x000006, 1*1024*1024, CRC(e89b66a2) SHA1(fce6e56d1759ffe987766426ecb28e9015a500b7) )
	ROM_LOAD16_BYTE( "128a12.12k", 0x400000, 512*1024, CRC(63de93e2) SHA1(c9a50e7beff1cbbc5d5820664adbd54d52782c54) )
	ROM_LOAD16_BYTE( "128a11.10k", 0x400001, 512*1024, CRC(4eac941a) SHA1(c0a33f4b975ebee217fd335001839992f4c0bdc8) )

	/* road generator */
	ROM_REGION( 0x40000, "gfx3", ROMREGION_ERASE00 )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "128a06.2d", 0x000000, 2*1024*1024, CRC(88ed598c) SHA1(3c123e26b3a12541df77b368bc0e0d486f5622b6) )
	ROM_LOAD( "128a07.1d", 0x200000, 2*1024*1024, CRC(db79a66e) SHA1(b7e118ed26bac557038e8ae6cb77f23f3da5646f) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "mystwarrj.nv", 0x0000, 0x080, CRC(8e259918) SHA1(5eb46b0d96278648e8d2e84304d9bccd8dd68430) )
ROM_END

ROM_START( mystwarra )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "128aaa01.20f", 0x000000, 0x40000, CRC(633ead86) SHA1(56d8628f6081e860c4c6109eabd1c1392f669996) )
	ROM_LOAD16_BYTE( "128aaa02.20g", 0x000001, 0x40000, CRC(69ab81a2) SHA1(545bc298dfc4de05bac15d63a84c10400231a04d) )
	ROM_LOAD16_BYTE( "128a03.19f",   0x100000, 0x80000, CRC(e98094f3) SHA1(a3f9b804ff487f792a00ce85a383868ab0b1b5d8) )
	ROM_LOAD16_BYTE( "128a04.19g",   0x100001, 0x80000, CRC(88c6a3e4) SHA1(7c2361f716a2320730a3dd6723a271e349ad61c3) )

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("128a05.6b", 0x000000, 0x020000, CRC(0e5194e0) SHA1(83356158d561f1b8e21f6ae5936b61da834a0545) )
	ROM_RELOAD(           0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x500000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "128a08.1h", 0x000000, 1*1024*1024, CRC(63d6cfa0) SHA1(324bf25cf79aa030d2dcc94a53c1984eb8abec3a) )
	ROM_LOADTILE_WORD( "128a09.1k", 0x000002, 1*1024*1024, CRC(573a7725) SHA1(f2fef32053ed2a65c6c3ddd3e1657a866aa80b3e) )
	ROM_LOADTILE_BYTE( "128a10.3h", 0x000004, 512*1024, CRC(558e545a) SHA1(cac53e545f3f8980d431443f2c3b8b95e6077d1c) )

	/* sprites */
	ROM_REGION( 0x500000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "128a16.22k", 0x000000, 1*1024*1024, CRC(459b6407) SHA1(e4dace4912f9558bee75a8e95ee2637f5e950b47) )
	ROM_LOAD64_WORD( "128a15.20k", 0x000002, 1*1024*1024, CRC(6bbfedf4) SHA1(0b3acb2b34c722ddc60c0e64e12baa1f225e4fbb) )
	ROM_LOAD64_WORD( "128a14.19k", 0x000004, 1*1024*1024, CRC(f7bd89dd) SHA1(c9b2ebd5a49840f8b260d53c25cfcc238d21c75c) )
	ROM_LOAD64_WORD( "128a13.17k", 0x000006, 1*1024*1024, CRC(e89b66a2) SHA1(fce6e56d1759ffe987766426ecb28e9015a500b7) )
	ROM_LOAD16_BYTE( "128a12.12k", 0x400000, 512*1024, CRC(63de93e2) SHA1(c9a50e7beff1cbbc5d5820664adbd54d52782c54) )
	ROM_LOAD16_BYTE( "128a11.10k", 0x400001, 512*1024, CRC(4eac941a) SHA1(c0a33f4b975ebee217fd335001839992f4c0bdc8) )

	/* road generator */
	ROM_REGION( 0x40000, "gfx3", ROMREGION_ERASE00 )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "128a06.2d", 0x000000, 2*1024*1024, CRC(88ed598c) SHA1(3c123e26b3a12541df77b368bc0e0d486f5622b6) )
	ROM_LOAD( "128a07.1d", 0x200000, 2*1024*1024, CRC(db79a66e) SHA1(b7e118ed26bac557038e8ae6cb77f23f3da5646f) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "mystwarra.nv", 0x0000, 0x080, CRC(38951263) SHA1(cc685188acf178efc4cea6eb596d6ba59f8fa420) )
ROM_END

ROM_START( viostorm )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE( "168eab01.15h", 0x000001, 0x80000, CRC(4eee6a8e) SHA1(5c83ed2011aa77f590abca4c469fdb565f35dde5) )
	ROM_LOAD16_BYTE( "168eab02.15f", 0x000000, 0x80000, CRC(8dd8aa4c) SHA1(e7937fe1272b635807ffff08a45a0338d48c376c) )

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("168a05.7c", 0x000000, 0x020000, CRC(507fb3eb) SHA1(a4f676e3caaafe86918c76ded08d0c202969adf6) )
	ROM_RELOAD(         0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x600000, "gfx1", ROMREGION_ERASE00)
	ROM_LOADTILE_WORD( "168a09.1h", 0x000000, 2*1024*1024, CRC(1b34a881) SHA1(5de20f7ee7f90d4f6dea349ca5000bfcf74253b1) )
	ROM_LOADTILE_WORD( "168a08.1k", 0x000002, 2*1024*1024, CRC(db0ce743) SHA1(dfe24a1e3e72da188a92668928e79afd6c5d22ee) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00)
	ROM_LOAD64_WORD( "168a10.22k", 0x000000, 2*1024*1024, CRC(bd2bbdea) SHA1(54faf2ded16e66d675bbbec4ebd42b4708edfaef) )
	ROM_LOAD64_WORD( "168a11.19k", 0x000002, 2*1024*1024, CRC(7a57c9e7) SHA1(8763c310f7b515aef52d4e007bc949e8803690f4) )
	ROM_LOAD64_WORD( "168a12.20k", 0x000004, 2*1024*1024, CRC(b6b1c4ef) SHA1(064ab4db884c8f98ab9e631b7034996d4b92ab7b) )
	ROM_LOAD64_WORD( "168a13.17k", 0x000006, 2*1024*1024, CRC(cdec3650) SHA1(949bc06bb38a2d5315ee4f6db19e043655b90e6e) )

	/* road generator */
	ROM_REGION( 0x40000, "gfx3", ROMREGION_ERASE00)

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0)
	ROM_LOAD( "168a06.1c", 0x000000, 2*1024*1024, CRC(25404fd7) SHA1(282cf523728b38d0bf14d765dd7257aa1fb2af39) )
	ROM_LOAD( "168a07.1e", 0x200000, 2*1024*1024, CRC(fdbbf8cc) SHA1(a8adf72a25fe2b9c4c338350d02c92deb5f8c8e9) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "viostorm.nv", 0x0000, 0x080, CRC(28b5fe49) SHA1(0ef51ae4b012a7d680543747fd4b6dd9dfb5f560) )
ROM_END

ROM_START( viostormu )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE( "168uac01.15h", 0x000001, 0x80000, CRC(49853530) SHA1(dc8fa1a929848949cb0ad02f5a2a8a5f820fd6c1) )
	ROM_LOAD16_BYTE( "168uac02.15f", 0x000000, 0x80000, CRC(055ca6fe) SHA1(31565ea515120555f94c4358b8e1a719c7d092d7) )

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("168a05.7c", 0x000000, 0x020000, CRC(507fb3eb) SHA1(a4f676e3caaafe86918c76ded08d0c202969adf6) )
	ROM_RELOAD(         0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x600000, "gfx1", ROMREGION_ERASE00)
	ROM_LOADTILE_WORD( "168a09.1h", 0x000000, 2*1024*1024, CRC(1b34a881) SHA1(5de20f7ee7f90d4f6dea349ca5000bfcf74253b1) )
	ROM_LOADTILE_WORD( "168a08.1k", 0x000002, 2*1024*1024, CRC(db0ce743) SHA1(dfe24a1e3e72da188a92668928e79afd6c5d22ee) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00)
	ROM_LOAD64_WORD( "168a10.22k", 0x000000, 2*1024*1024, CRC(bd2bbdea) SHA1(54faf2ded16e66d675bbbec4ebd42b4708edfaef) )
	ROM_LOAD64_WORD( "168a11.19k", 0x000002, 2*1024*1024, CRC(7a57c9e7) SHA1(8763c310f7b515aef52d4e007bc949e8803690f4) )
	ROM_LOAD64_WORD( "168a12.20k", 0x000004, 2*1024*1024, CRC(b6b1c4ef) SHA1(064ab4db884c8f98ab9e631b7034996d4b92ab7b) )
	ROM_LOAD64_WORD( "168a13.17k", 0x000006, 2*1024*1024, CRC(cdec3650) SHA1(949bc06bb38a2d5315ee4f6db19e043655b90e6e) )

	/* road generator */
	ROM_REGION( 0x40000, "gfx3", ROMREGION_ERASE00)

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0)
	ROM_LOAD( "168a06.1c", 0x000000, 2*1024*1024, CRC(25404fd7) SHA1(282cf523728b38d0bf14d765dd7257aa1fb2af39) )
	ROM_LOAD( "168a07.1e", 0x200000, 2*1024*1024, CRC(fdbbf8cc) SHA1(a8adf72a25fe2b9c4c338350d02c92deb5f8c8e9) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "viostormu.nv", 0x0000, 0x080, CRC(797042a1) SHA1(f33eb8b1b21f3d41372694fa6297bc2cc802c2eb) )
ROM_END

ROM_START( viostormub )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE( "168uab01.15h", 0x000001, 0x80000, CRC(2d6a9fa3) SHA1(a2f82702896eddb11cd2b2f9ed5fff730f6baf0f) )
	ROM_LOAD16_BYTE( "168uab02.15f", 0x000000, 0x80000, CRC(0e75f7cc) SHA1(57af86703dc728ba83ca12889246c93b9f8d4576) )

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("168a05.7c", 0x000000, 0x020000, CRC(507fb3eb) SHA1(a4f676e3caaafe86918c76ded08d0c202969adf6) )
	ROM_RELOAD(         0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x600000, "gfx1", ROMREGION_ERASE00)
	ROM_LOADTILE_WORD( "168a09.1h", 0x000000, 2*1024*1024, CRC(1b34a881) SHA1(5de20f7ee7f90d4f6dea349ca5000bfcf74253b1) )
	ROM_LOADTILE_WORD( "168a08.1k", 0x000002, 2*1024*1024, CRC(db0ce743) SHA1(dfe24a1e3e72da188a92668928e79afd6c5d22ee) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00)
	ROM_LOAD64_WORD( "168a10.22k", 0x000000, 2*1024*1024, CRC(bd2bbdea) SHA1(54faf2ded16e66d675bbbec4ebd42b4708edfaef) )
	ROM_LOAD64_WORD( "168a11.19k", 0x000002, 2*1024*1024, CRC(7a57c9e7) SHA1(8763c310f7b515aef52d4e007bc949e8803690f4) )
	ROM_LOAD64_WORD( "168a12.20k", 0x000004, 2*1024*1024, CRC(b6b1c4ef) SHA1(064ab4db884c8f98ab9e631b7034996d4b92ab7b) )
	ROM_LOAD64_WORD( "168a13.17k", 0x000006, 2*1024*1024, CRC(cdec3650) SHA1(949bc06bb38a2d5315ee4f6db19e043655b90e6e) )

	/* road generator */
	ROM_REGION( 0x40000, "gfx3", ROMREGION_ERASE00)

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0)
	ROM_LOAD( "168a06.1c", 0x000000, 2*1024*1024, CRC(25404fd7) SHA1(282cf523728b38d0bf14d765dd7257aa1fb2af39) )
	ROM_LOAD( "168a07.1e", 0x200000, 2*1024*1024, CRC(fdbbf8cc) SHA1(a8adf72a25fe2b9c4c338350d02c92deb5f8c8e9) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "viostormub.nv", 0x0000, 0x080, CRC(b6937413) SHA1(eabc2ea661201f5ed42ab541aee765480bbdd5bc) )
ROM_END

ROM_START( viostorma )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "168aac01.15h", 0x000001, 0x80000, CRC(3620635c) SHA1(d296ba707a131bd78b401608d6b165b214f4fe61) )
	ROM_LOAD16_BYTE( "168aac02.15f", 0x000000, 0x80000, CRC(db679aec) SHA1(233f3ab54125db1035cb0afadb06312ef7bd3e09) )

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("168a05.7c", 0x000000, 0x020000, CRC(507fb3eb) SHA1(a4f676e3caaafe86918c76ded08d0c202969adf6) )
	ROM_RELOAD(         0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x600000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "168a09.1h", 0x000000, 2*1024*1024, CRC(1b34a881) SHA1(5de20f7ee7f90d4f6dea349ca5000bfcf74253b1) )
	ROM_LOADTILE_WORD( "168a08.1k", 0x000002, 2*1024*1024, CRC(db0ce743) SHA1(dfe24a1e3e72da188a92668928e79afd6c5d22ee) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "168a10.22k", 0x000000, 2*1024*1024, CRC(bd2bbdea) SHA1(54faf2ded16e66d675bbbec4ebd42b4708edfaef) )
	ROM_LOAD64_WORD( "168a11.19k", 0x000002, 2*1024*1024, CRC(7a57c9e7) SHA1(8763c310f7b515aef52d4e007bc949e8803690f4) )
	ROM_LOAD64_WORD( "168a12.20k", 0x000004, 2*1024*1024, CRC(b6b1c4ef) SHA1(064ab4db884c8f98ab9e631b7034996d4b92ab7b) )
	ROM_LOAD64_WORD( "168a13.17k", 0x000006, 2*1024*1024, CRC(cdec3650) SHA1(949bc06bb38a2d5315ee4f6db19e043655b90e6e) )

	/* road generator */
	ROM_REGION( 0x40000, "gfx3", ROMREGION_ERASE00 )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "168a06.1c", 0x000000, 2*1024*1024, CRC(25404fd7) SHA1(282cf523728b38d0bf14d765dd7257aa1fb2af39) )
	ROM_LOAD( "168a07.1e", 0x200000, 2*1024*1024, CRC(fdbbf8cc) SHA1(a8adf72a25fe2b9c4c338350d02c92deb5f8c8e9) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "viostorma.nv", 0x0000, 0x080, CRC(2cfbf966) SHA1(fb9c4a47bac20a7f820a1fa178fc9f9079101cb8) )
ROM_END

ROM_START( viostormj )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "168jac01.b01", 0x000001, 0x80000, CRC(f8be1225) SHA1(8c38ca218c0005c60a48cd3a43b5460b63a851e7) )
	ROM_LOAD16_BYTE( "168jac02.b02", 0x000000, 0x80000, CRC(f42fd1e5) SHA1(3b17c3039d800487f6117595050e7896a413db04) )

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("168a05.7c", 0x000000, 0x020000, CRC(507fb3eb) SHA1(a4f676e3caaafe86918c76ded08d0c202969adf6) )
	ROM_RELOAD(         0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x600000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "168a09.1h", 0x000000, 2*1024*1024, CRC(1b34a881) SHA1(5de20f7ee7f90d4f6dea349ca5000bfcf74253b1) )
	ROM_LOADTILE_WORD( "168a08.1k", 0x000002, 2*1024*1024, CRC(db0ce743) SHA1(dfe24a1e3e72da188a92668928e79afd6c5d22ee) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "168a10.22k", 0x000000, 2*1024*1024, CRC(bd2bbdea) SHA1(54faf2ded16e66d675bbbec4ebd42b4708edfaef) )
	ROM_LOAD64_WORD( "168a11.19k", 0x000002, 2*1024*1024, CRC(7a57c9e7) SHA1(8763c310f7b515aef52d4e007bc949e8803690f4) )
	ROM_LOAD64_WORD( "168a12.20k", 0x000004, 2*1024*1024, CRC(b6b1c4ef) SHA1(064ab4db884c8f98ab9e631b7034996d4b92ab7b) )
	ROM_LOAD64_WORD( "168a13.17k", 0x000006, 2*1024*1024, CRC(cdec3650) SHA1(949bc06bb38a2d5315ee4f6db19e043655b90e6e) )

	/* road generator */
	ROM_REGION( 0x40000, "gfx3", ROMREGION_ERASE00 )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "168a06.1c", 0x000000, 2*1024*1024, CRC(25404fd7) SHA1(282cf523728b38d0bf14d765dd7257aa1fb2af39) )
	ROM_LOAD( "168a07.1e", 0x200000, 2*1024*1024, CRC(fdbbf8cc) SHA1(a8adf72a25fe2b9c4c338350d02c92deb5f8c8e9) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "viostormj.nv", 0x0000, 0x080, CRC(32f5d8bc) SHA1(0c486ce80b62bfaf4f3c0be7653c0beaf4cfafbd) )
ROM_END

ROM_START( metamrph )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "224eaa01.15h", 0x000001, 0x40000, CRC(30962c2b) SHA1(ae1b2f92881bb0f386b6a413c1da507e73c29382) )
	ROM_LOAD16_BYTE( "224eaa02.15f", 0x000000, 0x40000, CRC(e314330a) SHA1(728a18d604eca58409551e52b7dc18e2d807700a) )
	ROM_LOAD16_BYTE( "224a03",       0x100001, 0x80000, CRC(a5bedb01) SHA1(5e7a0b93af654ba6a87be8d449c7080a0f0e2a43) )
	ROM_LOAD16_BYTE( "224a04",       0x100000, 0x80000, CRC(ada53ba4) SHA1(f77bf854dff1f8f718579fe6d3730066708396e2) )

	/* sound program */
	ROM_REGION( 0x050000, "soundcpu", 0 )
	ROM_LOAD("224a05", 0x000000, 0x040000, CRC(4b4c985c) SHA1(c83cce05355023be9cd55b4aa595c61f8236269c) )
	ROM_RELOAD(           0x010000, 0x040000 )

	/* tiles */
	ROM_REGION( 0x500000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "224a09", 0x000000, 1*1024*1024, CRC(1931afce) SHA1(78838c0fd2a9c80f130db1fcf6c88b14f7363639) )
	ROM_LOADTILE_WORD( "224a08", 0x000002, 1*1024*1024, CRC(dc94d53a) SHA1(91e16371a335f078a81c06a1045759653080aba0) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "224a10", 0x000000, 2*1024*1024, CRC(161287f0) SHA1(a13b197a98fa1cebb11fb87b54e277c72852c4ee) )
	ROM_LOAD64_WORD( "224a11", 0x000002, 2*1024*1024, CRC(df5960e1) SHA1(ee7794dd119f5f2c52e7ba589d78067a89ff3cab) )
	ROM_LOAD64_WORD( "224a12", 0x000004, 2*1024*1024, CRC(ca72a4b3) SHA1(a09deb6d7cb8be4edaeb78e0e676ea2d6055e9e0) )
	ROM_LOAD64_WORD( "224a13", 0x000006, 2*1024*1024, CRC(86b58feb) SHA1(5a43746e2cd3c7aca21496c092aef83e64b3ab2c) )

	/* K053250 linescroll/zoom thingy */
	ROM_REGION( 0x80000, "gfx3", ROMREGION_ERASE00 ) // NOTE: region must be 2xROM size for unpacking
	ROM_LOAD( "224a14", 0x000000, 0x40000, CRC(3c79b404) SHA1(7c6bb4cbf050f314ea0cd3e8bc6e1947d0573084) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "224a06", 0x000000, 2*1024*1024, CRC(972f6abe) SHA1(30907495fc49fe3424c092b074c1dc137aa14306) )
	ROM_LOAD( "224a07", 0x200000, 1*1024*1024, CRC(61b2f97a) SHA1(34bf835d6361c7809d40fa20fd238c9e2a84b101) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "metamrph.nv", 0x0000, 0x080, CRC(2c51229a) SHA1(7f056792cc44ec3d4aacc33c825ab796a913488e) )
ROM_END

ROM_START( metamrphu )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "224uaa01.15h", 0x000001, 0x40000, CRC(e1d9b516) SHA1(387ed5ce87af376c0568e39187330a4585dc499a) )
	ROM_LOAD16_BYTE( "224uaa02.15f", 0x000000, 0x40000, CRC(289c926b) SHA1(5a5129fd85afc4fee97155e28bff89d3bb497b59) )
	ROM_LOAD16_BYTE( "224a03",       0x100001, 0x80000, CRC(a5bedb01) SHA1(5e7a0b93af654ba6a87be8d449c7080a0f0e2a43) )
	ROM_LOAD16_BYTE( "224a04",       0x100000, 0x80000, CRC(ada53ba4) SHA1(f77bf854dff1f8f718579fe6d3730066708396e2) )

	/* sound program */
	ROM_REGION( 0x050000, "soundcpu", 0 )
	ROM_LOAD("224a05", 0x000000, 0x040000, CRC(4b4c985c) SHA1(c83cce05355023be9cd55b4aa595c61f8236269c) )
	ROM_RELOAD(           0x010000, 0x040000 )

	/* tiles */
	ROM_REGION( 0x500000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "224a09", 0x000000, 1*1024*1024, CRC(1931afce) SHA1(78838c0fd2a9c80f130db1fcf6c88b14f7363639) )
	ROM_LOADTILE_WORD( "224a08", 0x000002, 1*1024*1024, CRC(dc94d53a) SHA1(91e16371a335f078a81c06a1045759653080aba0) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "224a10", 0x000000, 2*1024*1024, CRC(161287f0) SHA1(a13b197a98fa1cebb11fb87b54e277c72852c4ee) )
	ROM_LOAD64_WORD( "224a11", 0x000002, 2*1024*1024, CRC(df5960e1) SHA1(ee7794dd119f5f2c52e7ba589d78067a89ff3cab) )
	ROM_LOAD64_WORD( "224a12", 0x000004, 2*1024*1024, CRC(ca72a4b3) SHA1(a09deb6d7cb8be4edaeb78e0e676ea2d6055e9e0) )
	ROM_LOAD64_WORD( "224a13", 0x000006, 2*1024*1024, CRC(86b58feb) SHA1(5a43746e2cd3c7aca21496c092aef83e64b3ab2c) )

	/* K053250 linescroll/zoom thingy */
	ROM_REGION( 0x80000, "gfx3", 0 ) // NOTE: region must be 2xROM size for unpacking
	ROM_LOAD( "224a14", 0x000000, 0x40000, CRC(3c79b404) SHA1(7c6bb4cbf050f314ea0cd3e8bc6e1947d0573084) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "224a06", 0x000000, 2*1024*1024, CRC(972f6abe) SHA1(30907495fc49fe3424c092b074c1dc137aa14306) )
	ROM_LOAD( "224a07", 0x200000, 1*1024*1024, CRC(61b2f97a) SHA1(34bf835d6361c7809d40fa20fd238c9e2a84b101) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "metamrphu.nv", 0x0000, 0x080, CRC(1af2f855) SHA1(5f2fbb172f56867ee6f782cda8da65451b02435e) )
ROM_END

ROM_START( metamrphj )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "224jaa01.15h", 0x000001, 0x40000, CRC(558d2602) SHA1(2dbc16fcc07436ca7eff8d070196348f272b8723) )
	ROM_LOAD16_BYTE( "224jaa02.15f", 0x000000, 0x40000, CRC(9b252ace) SHA1(efe8cd942f3d4e2366d9af0fb9647d2a4aeac2c9) )
	ROM_LOAD16_BYTE( "224a03",       0x100001, 0x80000, CRC(a5bedb01) SHA1(5e7a0b93af654ba6a87be8d449c7080a0f0e2a43) )
	ROM_LOAD16_BYTE( "224a04",       0x100000, 0x80000, CRC(ada53ba4) SHA1(f77bf854dff1f8f718579fe6d3730066708396e2) )

	/* sound program */
	ROM_REGION( 0x050000, "soundcpu", 0 )
	ROM_LOAD("224a05", 0x000000, 0x040000, CRC(4b4c985c) SHA1(c83cce05355023be9cd55b4aa595c61f8236269c) )
	ROM_RELOAD(           0x010000, 0x040000 )

	/* tiles */
	ROM_REGION( 0x500000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "224a09", 0x000000, 1*1024*1024, CRC(1931afce) SHA1(78838c0fd2a9c80f130db1fcf6c88b14f7363639) )
	ROM_LOADTILE_WORD( "224a08", 0x000002, 1*1024*1024, CRC(dc94d53a) SHA1(91e16371a335f078a81c06a1045759653080aba0) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "224a10", 0x000000, 2*1024*1024, CRC(161287f0) SHA1(a13b197a98fa1cebb11fb87b54e277c72852c4ee) )
	ROM_LOAD64_WORD( "224a11", 0x000002, 2*1024*1024, CRC(df5960e1) SHA1(ee7794dd119f5f2c52e7ba589d78067a89ff3cab) )
	ROM_LOAD64_WORD( "224a12", 0x000004, 2*1024*1024, CRC(ca72a4b3) SHA1(a09deb6d7cb8be4edaeb78e0e676ea2d6055e9e0) )
	ROM_LOAD64_WORD( "224a13", 0x000006, 2*1024*1024, CRC(86b58feb) SHA1(5a43746e2cd3c7aca21496c092aef83e64b3ab2c) )

	/* K053250 linescroll/zoom thingy */
	ROM_REGION( 0x80000, "gfx3", 0 ) // NOTE: region must be 2xROM size for unpacking
	ROM_LOAD( "224a14", 0x000000, 0x40000, CRC(3c79b404) SHA1(7c6bb4cbf050f314ea0cd3e8bc6e1947d0573084) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "224a06", 0x000000, 2*1024*1024, CRC(972f6abe) SHA1(30907495fc49fe3424c092b074c1dc137aa14306) )
	ROM_LOAD( "224a07", 0x200000, 1*1024*1024, CRC(61b2f97a) SHA1(34bf835d6361c7809d40fa20fd238c9e2a84b101) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "metamrphj.nv", 0x0000, 0x080, CRC(30497478) SHA1(60acfbd25ac29c7b5a2571e274704205fc64424a) )
ROM_END

ROM_START( mtlchamp )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "234eab01.20f", 0x000000, 0x40000, CRC(7c4d1e50) SHA1(f943b955aa66f0892c20df7a94fc8bf070bf4624) )
	ROM_LOAD16_BYTE( "234eab02.20g", 0x000001, 0x40000, CRC(d8bc85c9) SHA1(40a13b4139dd24c06378c6bd982ef3f5fd646dcc) )
	ROM_LOAD16_BYTE( "234_d03.19f",  0x300000, 0x80000, CRC(abb577c6) SHA1(493f11a10a4d5b62d755ff8274e77d898544944f) )
	ROM_LOAD16_BYTE( "234_d04.19g",  0x300001, 0x80000, CRC(030a1925) SHA1(03783488950c9f27af5948e7b9f6a609c2df6e0b) )

	/* sound program */
	ROM_REGION( 0x040000, "soundcpu", 0 )
	ROM_LOAD("234_d05.6b", 0x000000, 0x020000, CRC(efb6bcaa) SHA1(4fb24b89a50b341871945547859278a6e2f5e002) )
	ROM_RELOAD(           0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x600000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "234a08.1h", 0x000000, 1*1024*1024, CRC(27e94288) SHA1(a92b03adf7beea6a1ceb74f659c87c628a7ab8e4) )
	ROM_LOADTILE_WORD( "234a09.1k", 0x000002, 1*1024*1024, CRC(03aad28f) SHA1(e7d9d788822ac9666e089b58288e3fcdba1b89da) )
	ROM_LOADTILE_BYTE( "234a10.3h", 0x000004, 512*1024, CRC(51f50fe2) SHA1(164fc975feff442d93f1917727c159051dcd3a55) )

	/* sprites */
	ROM_REGION( 0xa00000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "234a16.22k", 0x000000, 2*1024*1024, CRC(14d909a5) SHA1(15da356852fc0c63ecd924ac37ebe24bf3ba0760) )
	ROM_LOAD64_WORD( "234a15.20k", 0x000002, 2*1024*1024, CRC(a5028418) SHA1(ec6fc7b38fb1d27490a5a9310ecac2d1049e197c) )
	ROM_LOAD64_WORD( "234a14.19k", 0x000004, 2*1024*1024, CRC(d7921f47) SHA1(3fc97b308ad2ca25a376373ddfe08c8a375c424e) )
	ROM_LOAD64_WORD( "234a13.17k", 0x000006, 2*1024*1024, CRC(5974392e) SHA1(7c380419244439804797a9510846d273ebe99d02) )
	ROM_LOAD16_BYTE( "234a12.12k", 0x800000, 1024*1024, CRC(c7f2b099) SHA1(b72b80feb52560a5a42a1db39b059ac8bca27c10) )
	ROM_LOAD16_BYTE( "234a11.10k", 0x800001, 1024*1024, CRC(82923713) SHA1(a36cd3b2c9d36e93a3c25ba1d4e162f3d92e06ae) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "234a06.2d", 0x000000, 2*1024*1024, CRC(12d32384) SHA1(ecd6cd752b0e20339e17a7652ed843fbb43f7595) )
	ROM_LOAD( "234a07.1d", 0x200000, 2*1024*1024, CRC(05ee239f) SHA1(f4e6e7568dc73666a2b5e0c3fe743432e0436464) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "mtlchamp.nv", 0x0000, 0x080, CRC(cd47858e) SHA1(8effdcd631516d537f956509111cb3d4d18040db) )
ROM_END

ROM_START( mtlchamp1 )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "234eaa01.20f", 0x000000, 0x40000, CRC(8fa731db) SHA1(e2ed81762070a764f51aae36ce03859b5d86520d) )
	ROM_LOAD16_BYTE( "234eaa02.20g", 0x000001, 0x40000, CRC(e7b50b54) SHA1(594391a37479e6973aadd1fc866671c306a75497) )
	ROM_LOAD16_BYTE( "234_d03.19f",  0x300000, 0x80000, CRC(abb577c6) SHA1(493f11a10a4d5b62d755ff8274e77d898544944f) )
	ROM_LOAD16_BYTE( "234_d04.19g",  0x300001, 0x80000, CRC(030a1925) SHA1(03783488950c9f27af5948e7b9f6a609c2df6e0b) )

	/* sound program */
	ROM_REGION( 0x040000, "soundcpu", 0 )
	ROM_LOAD("234_d05.6b", 0x000000, 0x020000, CRC(efb6bcaa) SHA1(4fb24b89a50b341871945547859278a6e2f5e002) )
	ROM_RELOAD(           0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x600000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "234a08.1h", 0x000000, 1*1024*1024, CRC(27e94288) SHA1(a92b03adf7beea6a1ceb74f659c87c628a7ab8e4) )
	ROM_LOADTILE_WORD( "234a09.1k", 0x000002, 1*1024*1024, CRC(03aad28f) SHA1(e7d9d788822ac9666e089b58288e3fcdba1b89da) )
	ROM_LOADTILE_BYTE( "234a10.3h", 0x000004, 512*1024, CRC(51f50fe2) SHA1(164fc975feff442d93f1917727c159051dcd3a55) )

	/* sprites */
	ROM_REGION( 0xa00000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "234a16.22k", 0x000000, 2*1024*1024, CRC(14d909a5) SHA1(15da356852fc0c63ecd924ac37ebe24bf3ba0760) )
	ROM_LOAD64_WORD( "234a15.20k", 0x000002, 2*1024*1024, CRC(a5028418) SHA1(ec6fc7b38fb1d27490a5a9310ecac2d1049e197c) )
	ROM_LOAD64_WORD( "234a14.19k", 0x000004, 2*1024*1024, CRC(d7921f47) SHA1(3fc97b308ad2ca25a376373ddfe08c8a375c424e) )
	ROM_LOAD64_WORD( "234a13.17k", 0x000006, 2*1024*1024, CRC(5974392e) SHA1(7c380419244439804797a9510846d273ebe99d02) )
	ROM_LOAD16_BYTE( "234a12.12k", 0x800000, 1024*1024, CRC(c7f2b099) SHA1(b72b80feb52560a5a42a1db39b059ac8bca27c10) )
	ROM_LOAD16_BYTE( "234a11.10k", 0x800001, 1024*1024, CRC(82923713) SHA1(a36cd3b2c9d36e93a3c25ba1d4e162f3d92e06ae) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "234a06.2d", 0x000000, 2*1024*1024, CRC(12d32384) SHA1(ecd6cd752b0e20339e17a7652ed843fbb43f7595) )
	ROM_LOAD( "234a07.1d", 0x200000, 2*1024*1024, CRC(05ee239f) SHA1(f4e6e7568dc73666a2b5e0c3fe743432e0436464) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "mtlchamp1.nv", 0x0000, 0x080, CRC(202f6968) SHA1(38fc82a77896607c7fc09b75309f80048b52eb05) )
ROM_END

ROM_START( mtlchampa )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "234aaa01.20f", 0x000000, 0x40000, CRC(32c70e65) SHA1(10103ba403656d962bafe970a4ad0b2a9bed0ffe) )
	ROM_LOAD16_BYTE( "234aaa02.20g", 0x000001, 0x40000, CRC(2f666d52) SHA1(97765ef89e9b9ed36e7039f31f4d57187c6bb6e5) )
	ROM_LOAD16_BYTE( "234_d03.19f",  0x300000, 0x80000, CRC(abb577c6) SHA1(493f11a10a4d5b62d755ff8274e77d898544944f) )
	ROM_LOAD16_BYTE( "234_d04.19g",  0x300001, 0x80000, CRC(030a1925) SHA1(03783488950c9f27af5948e7b9f6a609c2df6e0b) )

	/* sound program */
	ROM_REGION( 0x040000, "soundcpu", 0 )
	ROM_LOAD("234_d05.6b", 0x000000, 0x020000, CRC(efb6bcaa) SHA1(4fb24b89a50b341871945547859278a6e2f5e002) )
	ROM_RELOAD(           0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x600000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "234a08.1h", 0x000000, 1*1024*1024, CRC(27e94288) SHA1(a92b03adf7beea6a1ceb74f659c87c628a7ab8e4) )
	ROM_LOADTILE_WORD( "234a09.1k", 0x000002, 1*1024*1024, CRC(03aad28f) SHA1(e7d9d788822ac9666e089b58288e3fcdba1b89da) )
	ROM_LOADTILE_BYTE( "234a10.3h", 0x000004, 512*1024, CRC(51f50fe2) SHA1(164fc975feff442d93f1917727c159051dcd3a55) )

	/* sprites */
	ROM_REGION( 0xa00000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "234a16.22k", 0x000000, 2*1024*1024, CRC(14d909a5) SHA1(15da356852fc0c63ecd924ac37ebe24bf3ba0760) )
	ROM_LOAD64_WORD( "234a15.20k", 0x000002, 2*1024*1024, CRC(a5028418) SHA1(ec6fc7b38fb1d27490a5a9310ecac2d1049e197c) )
	ROM_LOAD64_WORD( "234a14.19k", 0x000004, 2*1024*1024, CRC(d7921f47) SHA1(3fc97b308ad2ca25a376373ddfe08c8a375c424e) )
	ROM_LOAD64_WORD( "234a13.17k", 0x000006, 2*1024*1024, CRC(5974392e) SHA1(7c380419244439804797a9510846d273ebe99d02) )
	ROM_LOAD16_BYTE( "234a12.12k", 0x800000, 1024*1024, CRC(c7f2b099) SHA1(b72b80feb52560a5a42a1db39b059ac8bca27c10) )
	ROM_LOAD16_BYTE( "234a11.10k", 0x800001, 1024*1024, CRC(82923713) SHA1(a36cd3b2c9d36e93a3c25ba1d4e162f3d92e06ae) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "234a06.2d", 0x000000, 2*1024*1024, CRC(12d32384) SHA1(ecd6cd752b0e20339e17a7652ed843fbb43f7595) )
	ROM_LOAD( "234a07.1d", 0x200000, 2*1024*1024, CRC(05ee239f) SHA1(f4e6e7568dc73666a2b5e0c3fe743432e0436464) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "mtlchampa.nv", 0x0000, 0x080, CRC(79a6f420) SHA1(c2889bbb86a3f56d4f5544b6dadede0c715c59ca) )
ROM_END

ROM_START( mtlchampj )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "234jaa01.20f", 0x000000, 0x40000, CRC(76c3c568) SHA1(14c0009804fcedd8a3e5a105cc871dd6cd4cf7b3) )
	ROM_LOAD16_BYTE( "234jaa02.20g", 0x000001, 0x40000, CRC(95eec0aa) SHA1(11f1986d792951d6d5b3740b435dab2a2f4e6cbd) )
	ROM_LOAD16_BYTE( "234_d03.19f",  0x300000, 0x80000, CRC(abb577c6) SHA1(493f11a10a4d5b62d755ff8274e77d898544944f) )
	ROM_LOAD16_BYTE( "234_d04.19g",  0x300001, 0x80000, CRC(030a1925) SHA1(03783488950c9f27af5948e7b9f6a609c2df6e0b) )

	/* sound program */
	ROM_REGION( 0x040000, "soundcpu", 0 )
	ROM_LOAD("234_d05.6b", 0x000000, 0x020000, CRC(efb6bcaa) SHA1(4fb24b89a50b341871945547859278a6e2f5e002) )
	ROM_RELOAD(           0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x600000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "234a08.1h", 0x000000, 1*1024*1024, CRC(27e94288) SHA1(a92b03adf7beea6a1ceb74f659c87c628a7ab8e4) )
	ROM_LOADTILE_WORD( "234a09.1k", 0x000002, 1*1024*1024, CRC(03aad28f) SHA1(e7d9d788822ac9666e089b58288e3fcdba1b89da) )
	ROM_LOADTILE_BYTE( "234a10.3h", 0x000004, 512*1024, CRC(51f50fe2) SHA1(164fc975feff442d93f1917727c159051dcd3a55) )

	/* sprites */
	ROM_REGION( 0xa00000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "234a16.22k", 0x000000, 2*1024*1024, CRC(14d909a5) SHA1(15da356852fc0c63ecd924ac37ebe24bf3ba0760) )
	ROM_LOAD64_WORD( "234a15.20k", 0x000002, 2*1024*1024, CRC(a5028418) SHA1(ec6fc7b38fb1d27490a5a9310ecac2d1049e197c) )
	ROM_LOAD64_WORD( "234a14.19k", 0x000004, 2*1024*1024, CRC(d7921f47) SHA1(3fc97b308ad2ca25a376373ddfe08c8a375c424e) )
	ROM_LOAD64_WORD( "234a13.17k", 0x000006, 2*1024*1024, CRC(5974392e) SHA1(7c380419244439804797a9510846d273ebe99d02) )
	ROM_LOAD16_BYTE( "234a12.12k", 0x800000, 1024*1024, CRC(c7f2b099) SHA1(b72b80feb52560a5a42a1db39b059ac8bca27c10) )
	ROM_LOAD16_BYTE( "234a11.10k", 0x800001, 1024*1024, CRC(82923713) SHA1(a36cd3b2c9d36e93a3c25ba1d4e162f3d92e06ae) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "234a06.2d", 0x000000, 2*1024*1024, CRC(12d32384) SHA1(ecd6cd752b0e20339e17a7652ed843fbb43f7595) )
	ROM_LOAD( "234a07.1d", 0x200000, 2*1024*1024, CRC(05ee239f) SHA1(f4e6e7568dc73666a2b5e0c3fe743432e0436464) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "mtlchampj.nv", 0x0000, 0x080, CRC(e311816f) SHA1(1e8ece157e6b3978d11bc9a4a6015c6004f7a375) )
ROM_END

ROM_START( mtlchampu )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "234uad01.20f", 0x000000, 0x40000, CRC(5f6c8d09) SHA1(5850398cb9582973b400eaa82d84b7d07c87f779) )
	ROM_LOAD16_BYTE( "234uad02.20g", 0x000001, 0x40000, CRC(15ca4fb2) SHA1(f3025f0d54ce20717207ce219fd9e07b808eda34) )
	ROM_LOAD16_BYTE( "234_d03.19f",  0x300000, 0x80000, CRC(abb577c6) SHA1(493f11a10a4d5b62d755ff8274e77d898544944f) )
	ROM_LOAD16_BYTE( "234_d04.19g",  0x300001, 0x80000, CRC(030a1925) SHA1(03783488950c9f27af5948e7b9f6a609c2df6e0b) )

	/* sound program */
	ROM_REGION( 0x040000, "soundcpu", 0 )
	ROM_LOAD("234_d05.6b", 0x000000, 0x020000, CRC(efb6bcaa) SHA1(4fb24b89a50b341871945547859278a6e2f5e002) )
	ROM_RELOAD(           0x010000, 0x020000 )

	/* tiles */
	ROM_REGION( 0x600000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "234a08.1h", 0x000000, 1*1024*1024, CRC(27e94288) SHA1(a92b03adf7beea6a1ceb74f659c87c628a7ab8e4) )
	ROM_LOADTILE_WORD( "234a09.1k", 0x000002, 1*1024*1024, CRC(03aad28f) SHA1(e7d9d788822ac9666e089b58288e3fcdba1b89da) )
	ROM_LOADTILE_BYTE( "234a10.3h", 0x000004, 512*1024, CRC(51f50fe2) SHA1(164fc975feff442d93f1917727c159051dcd3a55) )

	/* sprites */
	ROM_REGION( 0xa00000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "234a16.22k", 0x000000, 2*1024*1024, CRC(14d909a5) SHA1(15da356852fc0c63ecd924ac37ebe24bf3ba0760) )
	ROM_LOAD64_WORD( "234a15.20k", 0x000002, 2*1024*1024, CRC(a5028418) SHA1(ec6fc7b38fb1d27490a5a9310ecac2d1049e197c) )
	ROM_LOAD64_WORD( "234a14.19k", 0x000004, 2*1024*1024, CRC(d7921f47) SHA1(3fc97b308ad2ca25a376373ddfe08c8a375c424e) )
	ROM_LOAD64_WORD( "234a13.17k", 0x000006, 2*1024*1024, CRC(5974392e) SHA1(7c380419244439804797a9510846d273ebe99d02) )
	ROM_LOAD16_BYTE( "234a12.12k", 0x800000, 1024*1024, CRC(c7f2b099) SHA1(b72b80feb52560a5a42a1db39b059ac8bca27c10) )
	ROM_LOAD16_BYTE( "234a11.10k", 0x800001, 1024*1024, CRC(82923713) SHA1(a36cd3b2c9d36e93a3c25ba1d4e162f3d92e06ae) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "234a06.2d", 0x000000, 2*1024*1024, CRC(12d32384) SHA1(ecd6cd752b0e20339e17a7652ed843fbb43f7595) )
	ROM_LOAD( "234a07.1d", 0x200000, 2*1024*1024, CRC(05ee239f) SHA1(f4e6e7568dc73666a2b5e0c3fe743432e0436464) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "mtlchampu.nv", 0x0000, 0x080, CRC(f5d84df7) SHA1(a14dca3ca275a754f1f46eab220a24b77ada23a5) )
ROM_END

ROM_START( gaiapols )
	/* main program */
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "123e07.24m", 0x000000, 1*1024*1024, CRC(f1a1db0f) SHA1(1394b8a57493cbc8d5bf555d363ad844a2407d98) )
	ROM_LOAD16_BYTE( "123e09.19l", 0x000001, 1*1024*1024, CRC(4b3b57e7) SHA1(1415ddc1393a468705d7a37d054ab6b08d0eb205) )

	/* 68k data */
	ROM_LOAD16_BYTE( "123eaf11.19p", 0x200000, 256*1024, CRC(9c324ade) SHA1(c78b7884f538d285b389c0c46d415a40da844d0d) )
	ROM_LOAD16_BYTE( "123eaf12.17p", 0x200001, 256*1024, CRC(1dfa14c5) SHA1(540700edbe3dbbd76e3b9e2d2acc416940730a9b) )

	/* sound program */
	ROM_REGION( 0x050000, "soundcpu", 0 )
	ROM_LOAD("123e13.9c", 0x000000, 0x040000, CRC(e772f822) SHA1(2a5cdfc0aacad56cbef8bdbe8319e7ff4ab71eee) )
	ROM_RELOAD(           0x010000, 0x040000 )

	/* tiles */
	ROM_REGION( 0x500000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "123e16.2t", 0x000000, 1*1024*1024, CRC(a3238200) SHA1(9ea073f7611b5c0b328c707c97ef174634c303bc) )
	ROM_LOADTILE_WORD( "123e17.2x", 0x000002, 1*1024*1024, CRC(bd0b9fb9) SHA1(1714c19d2123b8fd4cfedb66d13bdcee6fc77576) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "123e19.34u", 0x000000, 2*1024*1024, CRC(219a7c26) SHA1(0f24873523f91379862e0d4317fc42b8a10d412e) )
	ROM_LOAD64_WORD( "123e21.34y", 0x000002, 2*1024*1024, CRC(1888947b) SHA1(41a046cbcd2406add2ff48cb82a1353d9ac5fa3c) )
	ROM_LOAD64_WORD( "123e18.36u", 0x000004, 2*1024*1024, CRC(3719b6d4) SHA1(25ef6e8c3c7f151f1cee063356bdd56c713356ea) )
	ROM_LOAD64_WORD( "123e20.36y", 0x000006, 2*1024*1024, CRC(490a6f64) SHA1(81b1acc74ce834399005a748eae50b3d633fa469) )

	/* K053536 roz tiles */
	ROM_REGION( 0x180000, "gfx3", 0 )
	ROM_LOAD( "123e04.32n", 0x000000, 0x080000, CRC(0d4d5b8b) SHA1(d3fb0c77ad46ee9b9c704be6f174258aa051aa71) )
	ROM_LOAD( "123e05.29n", 0x080000, 0x080000, CRC(7d123f3e) SHA1(f9752e96515dc965aae04e01dfa813fcc4cbccd6) )
	ROM_LOAD( "123e06.26n", 0x100000, 0x080000, CRC(fa50121e) SHA1(4596a9b0a6cc67f259182098d3976234b6ed8cb6) )

	/* K053936 map data */
	ROM_REGION( 0xa0000, "gfx4", 0 )
	ROM_LOAD( "123e01.36j", 0x000000, 0x20000, CRC(9dbc9678) SHA1(4183eb833d0d1cd710fac32071df7ebcb7a9c812) )
	ROM_LOAD( "123e02.34j", 0x020000, 0x40000, CRC(b8e3f500) SHA1(254c665b7aa534990e899fe8f54c3f24e8126fba) )
	ROM_LOAD( "123e03.36m", 0x060000, 0x40000, CRC(fde4749f) SHA1(7f9c09d11dcb16d72046c7605570c3a29e279fa9) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "123e14.2g", 0x000000, 2*1024*1024, CRC(65dfd3ff) SHA1(57e13c05f420747c1c2010cc5340dd70e2c28971) )
	ROM_LOAD( "123e15.2m", 0x200000, 2*1024*1024, CRC(7017ff07) SHA1(37ecd54f2c757c5385305ab726d9f66aa1afd456) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "gaiapols.nv", 0x0000, 0x080, CRC(44c78184) SHA1(19343b47b60bf4e212d844fce28e7a1bd54c7012) )
ROM_END

ROM_START( gaiapolsu )
	/* main program */
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "123e07.24m", 0x000000, 1*1024*1024, CRC(f1a1db0f) SHA1(1394b8a57493cbc8d5bf555d363ad844a2407d98) )
	ROM_LOAD16_BYTE( "123e09.19l", 0x000001, 1*1024*1024, CRC(4b3b57e7) SHA1(1415ddc1393a468705d7a37d054ab6b08d0eb205) )

	/* 68k data */
	ROM_LOAD16_BYTE( "123uaf11.19p", 0x200000, 256*1024, CRC(39dc1298) SHA1(ce9e41ac6f52e20f13ad86fb0d47c0d6e838250e) )
	ROM_LOAD16_BYTE( "123uaf12.17p", 0x200001, 256*1024, CRC(c633cf52) SHA1(370be5557a271699342b7d771ebadf7021a27ae3) )

	/* sound program */
	ROM_REGION( 0x050000, "soundcpu", 0 )
	ROM_LOAD("123e13.9c", 0x000000, 0x040000, CRC(e772f822) SHA1(2a5cdfc0aacad56cbef8bdbe8319e7ff4ab71eee) )
	ROM_RELOAD(           0x010000, 0x040000 )

	/* tiles */
	ROM_REGION( 0x500000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "123e16.2t", 0x000000, 1*1024*1024, CRC(a3238200) SHA1(9ea073f7611b5c0b328c707c97ef174634c303bc) )
	ROM_LOADTILE_WORD( "123e17.2x", 0x000002, 1*1024*1024, CRC(bd0b9fb9) SHA1(1714c19d2123b8fd4cfedb66d13bdcee6fc77576) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "123e19.34u", 0x000000, 2*1024*1024, CRC(219a7c26) SHA1(0f24873523f91379862e0d4317fc42b8a10d412e) )
	ROM_LOAD64_WORD( "123e21.34y", 0x000002, 2*1024*1024, CRC(1888947b) SHA1(41a046cbcd2406add2ff48cb82a1353d9ac5fa3c) )
	ROM_LOAD64_WORD( "123e18.36u", 0x000004, 2*1024*1024, CRC(3719b6d4) SHA1(25ef6e8c3c7f151f1cee063356bdd56c713356ea) )
	ROM_LOAD64_WORD( "123e20.36y", 0x000006, 2*1024*1024, CRC(490a6f64) SHA1(81b1acc74ce834399005a748eae50b3d633fa469) )

	/* K053536 roz tiles */
	ROM_REGION( 0x180000, "gfx3", 0 )
	ROM_LOAD( "123e04.32n", 0x000000, 0x080000, CRC(0d4d5b8b) SHA1(d3fb0c77ad46ee9b9c704be6f174258aa051aa71) )
	ROM_LOAD( "123e05.29n", 0x080000, 0x080000, CRC(7d123f3e) SHA1(f9752e96515dc965aae04e01dfa813fcc4cbccd6) )
	ROM_LOAD( "123e06.26n", 0x100000, 0x080000, CRC(fa50121e) SHA1(4596a9b0a6cc67f259182098d3976234b6ed8cb6) )

	/* K053936 map data */
	ROM_REGION( 0xa0000, "gfx4", 0 )
	ROM_LOAD( "123e01.36j", 0x000000, 0x20000, CRC(9dbc9678) SHA1(4183eb833d0d1cd710fac32071df7ebcb7a9c812) )
	ROM_LOAD( "123e02.34j", 0x020000, 0x40000, CRC(b8e3f500) SHA1(254c665b7aa534990e899fe8f54c3f24e8126fba) )
	ROM_LOAD( "123e03.36m", 0x060000, 0x40000, CRC(fde4749f) SHA1(7f9c09d11dcb16d72046c7605570c3a29e279fa9) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "123e14.2g", 0x000000, 2*1024*1024, CRC(65dfd3ff) SHA1(57e13c05f420747c1c2010cc5340dd70e2c28971) )
	ROM_LOAD( "123e15.2m", 0x200000, 2*1024*1024, CRC(7017ff07) SHA1(37ecd54f2c757c5385305ab726d9f66aa1afd456) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "gaiapolsu.nv", 0x0000, 0x080, CRC(7ece27b6) SHA1(f0671c5e6db665c86afcef563ff1dbcbf083b380) )
ROM_END

ROM_START( gaiapolsj )
	/* main program */
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "123e07.24m", 0x000000, 1*1024*1024, CRC(f1a1db0f) SHA1(1394b8a57493cbc8d5bf555d363ad844a2407d98) )
	ROM_LOAD16_BYTE( "123e09.19l", 0x000001, 1*1024*1024, CRC(4b3b57e7) SHA1(1415ddc1393a468705d7a37d054ab6b08d0eb205) )

	/* 68k data */
	ROM_LOAD16_BYTE( "123jaf11.19p", 0x200000, 256*1024, CRC(19919571) SHA1(e4fbbdd4003f18631e5723bb85a7fa60e57f2d2a) )
	ROM_LOAD16_BYTE( "123jaf12.17p", 0x200001, 256*1024, CRC(4246e595) SHA1(d5fe0b1dfe2a0c64b3e62820dea5094cc0f5bd12) )

	/* sound program */
	ROM_REGION( 0x050000, "soundcpu", 0 )
	ROM_LOAD("123e13.9c", 0x000000, 0x040000, CRC(e772f822) SHA1(2a5cdfc0aacad56cbef8bdbe8319e7ff4ab71eee) )
	ROM_RELOAD(           0x010000, 0x040000 )

	/* tiles */
	ROM_REGION( 0x500000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "123e16.2t", 0x000000, 1*1024*1024, CRC(a3238200) SHA1(9ea073f7611b5c0b328c707c97ef174634c303bc) )
	ROM_LOADTILE_WORD( "123e17.2x", 0x000002, 1*1024*1024, CRC(bd0b9fb9) SHA1(1714c19d2123b8fd4cfedb66d13bdcee6fc77576) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "123e19.34u", 0x000000, 2*1024*1024, CRC(219a7c26) SHA1(0f24873523f91379862e0d4317fc42b8a10d412e) )
	ROM_LOAD64_WORD( "123e21.34y", 0x000002, 2*1024*1024, CRC(1888947b) SHA1(41a046cbcd2406add2ff48cb82a1353d9ac5fa3c) )
	ROM_LOAD64_WORD( "123e18.36u", 0x000004, 2*1024*1024, CRC(3719b6d4) SHA1(25ef6e8c3c7f151f1cee063356bdd56c713356ea) )
	ROM_LOAD64_WORD( "123e20.36y", 0x000006, 2*1024*1024, CRC(490a6f64) SHA1(81b1acc74ce834399005a748eae50b3d633fa469) )

	/* K053536 roz tiles */
	ROM_REGION( 0x180000, "gfx3", 0 )
	ROM_LOAD( "123e04.32n", 0x000000, 0x080000, CRC(0d4d5b8b) SHA1(d3fb0c77ad46ee9b9c704be6f174258aa051aa71) )
	ROM_LOAD( "123e05.29n", 0x080000, 0x080000, CRC(7d123f3e) SHA1(f9752e96515dc965aae04e01dfa813fcc4cbccd6) )
	ROM_LOAD( "123e06.26n", 0x100000, 0x080000, CRC(fa50121e) SHA1(4596a9b0a6cc67f259182098d3976234b6ed8cb6) )

	/* K053936 map data */
	ROM_REGION( 0xa0000, "gfx4", 0 )
	ROM_LOAD( "123e01.36j", 0x000000, 0x20000, CRC(9dbc9678) SHA1(4183eb833d0d1cd710fac32071df7ebcb7a9c812) )
	ROM_LOAD( "123e02.34j", 0x020000, 0x40000, CRC(b8e3f500) SHA1(254c665b7aa534990e899fe8f54c3f24e8126fba) )
	ROM_LOAD( "123e03.36m", 0x060000, 0x40000, CRC(fde4749f) SHA1(7f9c09d11dcb16d72046c7605570c3a29e279fa9) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "123e14.2g", 0x000000, 2*1024*1024, CRC(65dfd3ff) SHA1(57e13c05f420747c1c2010cc5340dd70e2c28971) )
	ROM_LOAD( "123e15.2m", 0x200000, 2*1024*1024, CRC(7017ff07) SHA1(37ecd54f2c757c5385305ab726d9f66aa1afd456) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "gaiapolsj.nv", 0x0000, 0x080, CRC(c4b970df) SHA1(d4a24b4950ee33a832342c752c24b58e033d9240) )
ROM_END

ROM_START( mmaulers )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "170eaa07.24m", 0x000000, 0x80000, CRC(5458bd93) SHA1(d27a29076be3c745e3efdb1c528b07bd5d8aff1c) )
	ROM_LOAD16_BYTE( "170eaa09.19l", 0x000001, 0x80000, CRC(99c95c7b) SHA1(7f22930c2fe21205ccd01b80566d6bc31fea34d2) )
	ROM_LOAD16_BYTE( "170a08.21m",   0x100000, 0x40000, CRC(03c59ba2) SHA1(041473fe5f9004bfb7ca767c2004154c27f726ff) )
	ROM_LOAD16_BYTE( "170a10.17l",   0x100001, 0x40000, CRC(8a340909) SHA1(3e2ef2642e792cdc38b3442df67377ed9e70d3ab) )

	/* sound program */
	ROM_REGION( 0x080000, "soundcpu", 0 )
	ROM_LOAD("170a13.9c", 0x000000, 0x40000, CRC(2ebf4d1c) SHA1(33a3f4153dfdc46cc223d216a17ef9428c09129d) )
	ROM_RELOAD(           0x010000, 0x040000 )

	/* tiles */
	ROM_REGION( 0x600000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "170a16.2t", 0x000000, 1*1024*1024, CRC(41fee912) SHA1(73cf167ac9fc42cb8048a87b6c6d1c3c0ae3c2e2) )
	ROM_LOADTILE_WORD( "170a17.2x", 0x000002, 1*1024*1024, CRC(96957c91) SHA1(b12d356f8a015ec0984bdb86da9c569eb0c67880) )
	ROM_LOADTILE_BYTE( "170a24.5r", 0x000004, 512*1024, CRC(562ad4bd) SHA1(f55b29142ea39f090244f0945a56760bab25c7a7) )

	/* sprites */
	ROM_REGION( 0xa00000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "170a19.34u", 0x000000, 2*1024*1024, CRC(be835141) SHA1(b76e1da45bf602dd9eb30fb8b7181cea2e820c3d) )
	ROM_LOAD64_WORD( "170a21.34y", 0x000002, 2*1024*1024, CRC(bcb68136) SHA1(1d453f59d832b8ea99cf0a60a917edce5c1c90a0) )
	ROM_LOAD64_WORD( "170a18.36u", 0x000004, 2*1024*1024, CRC(e1e3c8d2) SHA1(2c94fcedd1dcef3d3332af358ae8a67dea507216) )
	ROM_LOAD64_WORD( "170a20.36y", 0x000006, 2*1024*1024, CRC(ccb4d88c) SHA1(064b4dab0ca6e5a1fa2fc2e9bbb19c7499830ee1) )
	ROM_LOAD16_BYTE( "170a23.29y", 0x800000, 1024*1024, CRC(6b5390e4) SHA1(0c5066bc86e782db4b64c2a604aed89ae99af005) )
	ROM_LOAD16_BYTE( "170a22.32y", 0x800001, 1024*1024, CRC(21628106) SHA1(1e025ff53caa5cbbf7695f8a77736d59f8a8af1b) )

	/* K053536 roz plane */
	ROM_REGION( 0x180000, "gfx3", 0 )
	ROM_LOAD( "170a04.33n", 0x000000, 0x80000, CRC(64b9a73b) SHA1(8b984bfd8bdf6d93ad223fca46a4f958a0edb2be) )
	ROM_LOAD( "170a05.30n", 0x080000, 0x80000, CRC(f2c101d0) SHA1(d80045c9a02db08ea6c851bdc12826862e11c381) )
	ROM_LOAD( "170a06.27n", 0x100000, 0x80000, CRC(b032e59b) SHA1(482300c683db20c2b2fc6e007b8f7e35373e3c00) )

	/* K053936 tilemap data */
	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "170a02.34j", 0x000000, 0x40000, CRC(b040cebf) SHA1(4d1ba4ee60fd7caf678837ec6f4d68fcbce1ccf2) )
	ROM_LOAD( "170a03.36m", 0x040000, 0x40000, CRC(7fb412b2) SHA1(f603a8f0becf88e345f4b7a68cf018962a255a1e) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD("170a14.2g", 0x000000, 2*1024*1024, CRC(83317cda) SHA1(c5398c5959ef3ea73835e13db69660dd28c31486) )
	ROM_LOAD("170a15.2m", 0x200000, 2*1024*1024, CRC(d4113ae9) SHA1(e234d06f462e3db64455c384c2f42174f9ef9c6a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "mmaulers.nv", 0x0000, 0x080, CRC(8324f517) SHA1(4697d091a1924e1a5d6c3ffc64a40fd36eebe557) )
ROM_END

ROM_START( dadandrn )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "170jaa07.24m", 0x000000, 0x80000, CRC(6a55e828) SHA1(06be9a45fdddcc826a33ff8d6eb595e11b98a31f) )
	ROM_LOAD16_BYTE( "170jaa09.19l", 0x000001, 0x80000, CRC(9e821cd8) SHA1(51e9c3b0a187db62cfcdff23ecaf3205f368f4e0) )
	ROM_LOAD16_BYTE( "170a08.21m",   0x100000, 0x40000, CRC(03c59ba2) SHA1(041473fe5f9004bfb7ca767c2004154c27f726ff) )
	ROM_LOAD16_BYTE( "170a10.17l",   0x100001, 0x40000, CRC(8a340909) SHA1(3e2ef2642e792cdc38b3442df67377ed9e70d3ab) )

	/* sound program */
	ROM_REGION( 0x080000, "soundcpu", 0 )
	ROM_LOAD("170a13.9c", 0x000000, 0x40000, CRC(2ebf4d1c) SHA1(33a3f4153dfdc46cc223d216a17ef9428c09129d) )
	ROM_RELOAD(           0x010000, 0x040000 )

	/* tiles */
	ROM_REGION( 0x600000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOADTILE_WORD( "170a16.2t", 0x000000, 1*1024*1024, CRC(41fee912) SHA1(73cf167ac9fc42cb8048a87b6c6d1c3c0ae3c2e2) )
	ROM_LOADTILE_WORD( "170a17.2x", 0x000002, 1*1024*1024, CRC(96957c91) SHA1(b12d356f8a015ec0984bdb86da9c569eb0c67880) )
	ROM_LOADTILE_BYTE( "170a24.5r", 0x000004, 512*1024, CRC(562ad4bd) SHA1(f55b29142ea39f090244f0945a56760bab25c7a7) )

	/* sprites */
	ROM_REGION( 0xa00000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD64_WORD( "170a19.34u", 0x000000, 2*1024*1024, CRC(be835141) SHA1(b76e1da45bf602dd9eb30fb8b7181cea2e820c3d) )
	ROM_LOAD64_WORD( "170a21.34y", 0x000002, 2*1024*1024, CRC(bcb68136) SHA1(1d453f59d832b8ea99cf0a60a917edce5c1c90a0) )
	ROM_LOAD64_WORD( "170a18.36u", 0x000004, 2*1024*1024, CRC(e1e3c8d2) SHA1(2c94fcedd1dcef3d3332af358ae8a67dea507216) )
	ROM_LOAD64_WORD( "170a20.36y", 0x000006, 2*1024*1024, CRC(ccb4d88c) SHA1(064b4dab0ca6e5a1fa2fc2e9bbb19c7499830ee1) )
	ROM_LOAD16_BYTE( "170a23.29y", 0x800000, 1024*1024, CRC(6b5390e4) SHA1(0c5066bc86e782db4b64c2a604aed89ae99af005) )
	ROM_LOAD16_BYTE( "170a22.32y", 0x800001, 1024*1024, CRC(21628106) SHA1(1e025ff53caa5cbbf7695f8a77736d59f8a8af1b) )

	/* K053536 roz plane */
	ROM_REGION( 0x180000, "gfx3", 0 )
	ROM_LOAD( "170a04.33n", 0x000000, 0x80000, CRC(64b9a73b) SHA1(8b984bfd8bdf6d93ad223fca46a4f958a0edb2be) )
	ROM_LOAD( "170a05.30n", 0x080000, 0x80000, CRC(f2c101d0) SHA1(d80045c9a02db08ea6c851bdc12826862e11c381) )
	ROM_LOAD( "170a06.27n", 0x100000, 0x80000, CRC(b032e59b) SHA1(482300c683db20c2b2fc6e007b8f7e35373e3c00) )

	/* K053936 tilemap data */
	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "170a02.34j", 0x000000, 0x40000, CRC(b040cebf) SHA1(4d1ba4ee60fd7caf678837ec6f4d68fcbce1ccf2) )
	ROM_LOAD( "170a03.36m", 0x040000, 0x40000, CRC(7fb412b2) SHA1(f603a8f0becf88e345f4b7a68cf018962a255a1e) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD("170a14.2g", 0x000000, 2*1024*1024, CRC(83317cda) SHA1(c5398c5959ef3ea73835e13db69660dd28c31486) )
	ROM_LOAD("170a15.2m", 0x200000, 2*1024*1024, CRC(d4113ae9) SHA1(e234d06f462e3db64455c384c2f42174f9ef9c6a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "dadandrn.nv", 0x0000, 0x080, CRC(346ae0cf) SHA1(1f79b2e21766f7a971c7d0f618700deb8a32f78a) )
ROM_END

static DRIVER_INIT(metamrph)
{
	K053250_unpack_pixels(machine, "gfx3");
}


/*           ROM       parent    machine   inp       init */
GAME( 1993, mystwarr,  0,        mystwarr, mystwarr, 0,        ROT0,  "Konami", "Mystic Warriors (ver EAA)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, mystwarru, mystwarr, mystwarr, mystwarr, 0,        ROT0,  "Konami", "Mystic Warriors (ver UAA)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, mystwarrj, mystwarr, mystwarr, mystwarr, 0,        ROT0,  "Konami", "Mystic Warriors (ver JAA)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, mystwarra, mystwarr, mystwarr, mystwarr, 0,        ROT0,  "Konami", "Mystic Warriors (ver AAA)", GAME_IMPERFECT_GRAPHICS )

GAME( 1993, mmaulers,  0,        dadandrn, dadandrn, 0,        ROT0,  "Konami", "Monster Maulers (ver EAA)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, dadandrn,  mmaulers, dadandrn, dadandrn, 0,        ROT0,  "Konami", "Kyukyoku Sentai Dadandarn (ver JAA)", GAME_IMPERFECT_GRAPHICS )

GAME( 1993, viostorm,  0,        viostorm, viostorm, 0,        ROT0,  "Konami", "Violent Storm (ver EAB)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, viostormu, viostorm, viostorm, viostorm, 0,        ROT0,  "Konami", "Violent Storm (ver UAC)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, viostormub,viostorm, viostorm, viostorm, 0,        ROT0,  "Konami", "Violent Storm (ver UAB)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, viostormj, viostorm, viostorm, viostorm, 0,        ROT0,  "Konami", "Violent Storm (ver JAC)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, viostorma, viostorm, viostorm, viostorm, 0,        ROT0,  "Konami", "Violent Storm (ver AAC)", GAME_IMPERFECT_GRAPHICS )

GAME( 1993, metamrph,  0,        metamrph, metamrph, metamrph, ROT0,  "Konami", "Metamorphic Force (ver EAA)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, metamrphu, metamrph, metamrph, metamrph, metamrph, ROT0,  "Konami", "Metamorphic Force (ver UAA)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, metamrphj, metamrph, metamrph, metamrph, metamrph, ROT0,  "Konami", "Metamorphic Force (ver JAA)", GAME_IMPERFECT_GRAPHICS )

GAME( 1993, mtlchamp,  0,        martchmp, martchmp, 0,        ROT0,  "Konami", "Martial Champion (ver EAB)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, mtlchamp1, mtlchamp, martchmp, martchmp, 0,        ROT0,  "Konami", "Martial Champion (ver EAA)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, mtlchampu, mtlchamp, martchmp, martchmp, 0,        ROT0,  "Konami", "Martial Champion (ver UAD)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, mtlchampj, mtlchamp, martchmp, martchmp, 0,        ROT0,  "Konami", "Martial Champion (ver JAA)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, mtlchampa, mtlchamp, martchmp, martchmp, 0,        ROT0,  "Konami", "Martial Champion (ver AAA)", GAME_IMPERFECT_GRAPHICS )

GAME( 1993, gaiapols,  0,        gaiapols, dadandrn, 0,        ROT90, "Konami", "Gaiapolis (ver EAF)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, gaiapolsu, gaiapols, gaiapols, dadandrn, 0,        ROT90, "Konami", "Gaiapolis (ver UAF)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, gaiapolsj, gaiapols, gaiapols, dadandrn, 0,        ROT90, "Konami", "Gaiapolis (ver JAF)", GAME_IMPERFECT_GRAPHICS )
