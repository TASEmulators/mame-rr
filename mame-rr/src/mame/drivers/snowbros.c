/***************************************************************************

  Snow Brothers (Toaplan) / SemiCom Hardware
  uses Kaneko's Pandora sprite chip (also used in DJ Boy, Air Buster ..)

Snow Bros Nick & Tom
Toaplan, 1990

    PCB Layout
    ----------
    MIN16-02

    |------------------------------------------|
    | VOL     YM3812  6116  4464  4464         |
    | LA4460  YM3014        4464  4464         |
    |       458   SBROS-4.29         SBROS1.40 |
    |   2003       Z80B     PANDORA            |
    |J                    D41101C-1 LS07  LS32 |
    |A SBROS-3A.5 SBROS-2A.6        LS139 LS174|
    |M                  LS245 LS74  LS04  16MHz|
    |M   6264     6264  F32   LS74  LS74       |
    |A      68000       LS20  F138  LS04  12MHz|
    |                   LS04  LS148 LS251 LS00 |
    | LS273 LS245 LS245 LS158 LS257 LS257 LS32 |
    |                                          |
    | LS273  6116  6116 LS157   DSW2  DSW1     |
    |------------------------------------------|

    Notes:
           68k clock: 8.000MHz
          Z80B clock: 6.000MHz
        YM3812 clock: 3.000MHz
               VSync: 57.5Hz
               HSync: 15.68kHz

  driver by Mike Coates

  Hyper Pacman addition by David Haywood
   + some bits by Nicola Salmoria


Stephh's notes (hyperpac):

  - According to the "Language" Dip Switch, this game is a Korean game.
     (although the Language Dipswitch doesn't affect language, but yes
      I believe SemiCom to be a Korean Company)
  - There is no "cocktail mode", nor way to flip the screen.

Notes:

Cookie & Bibi 3
This game is quite buggy.  The test mode is incomplete and displays garbage
on the 'Dipswitch settings' screens, and during some of the attract mode
scenes the credit counter is not updated when you insert coins until the next
scene.  Both these bugs are verified as occuring on the original hardware.

Honey Doll / Twin Adventure

These appear to have clipping problems on the left / right edges, but this
may be correct, the sprites which should be drawn there are simply blanked
out of the sprite list at that point.. (verify on real hw)

***************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "video/kan_pand.h" // for the original pandora
#include "video/kan_panb.h" // for bootlegs / non-original hw
#include "cpu/mcs51/mcs51.h" // for semicom mcu

static WRITE16_HANDLER( snowbros_flipscreen_w )
{
	if (ACCESSING_BITS_8_15)
		flip_screen_set(space->machine, ~data & 0x8000);
}


static VIDEO_UPDATE( snowbros )
{
	running_device *pandora = screen->machine->device("pandora");

	/* This clears & redraws the entire screen each pass */
	bitmap_fill(bitmap,cliprect,0xf0);
	pandora_update(pandora, bitmap, cliprect);
	return 0;
}


static VIDEO_EOF( snowbros )
{
	running_device *pandora = machine->device("pandora");
	pandora_eof(pandora);
}


static UINT16 *hyperpac_ram;
static int sb3_music_is_playing;
static int sb3_music;

static WRITE16_HANDLER( snowbros_irq4_ack_w )
{
	cputag_set_input_line(space->machine, "maincpu", 4, CLEAR_LINE);
}

static WRITE16_HANDLER( snowbros_irq3_ack_w )
{
	cputag_set_input_line(space->machine, "maincpu", 3, CLEAR_LINE);
}

static WRITE16_HANDLER( snowbros_irq2_ack_w )
{
	cputag_set_input_line(space->machine, "maincpu", 2, CLEAR_LINE);
}

static INTERRUPT_GEN( snowbros_interrupt )
{
	cpu_set_input_line(device, cpu_getiloops(device) + 2, ASSERT_LINE);	/* IRQs 4, 3, and 2 */
}

static INTERRUPT_GEN( snowbro3_interrupt )
{
	running_device *adpcm = device->machine->device("oki");
	int status = okim6295_r(adpcm,0);

	cpu_set_input_line(device, cpu_getiloops(device) + 2, ASSERT_LINE);	/* IRQs 4, 3, and 2 */

	if (sb3_music_is_playing)
	{
		if ((status&0x08)==0x00)
		{
			okim6295_w(adpcm,0,0x80|sb3_music);
			okim6295_w(adpcm,0,0x00|0x82);
		}

	}
	else
	{
		if ((status&0x08)==0x08)
		{
			okim6295_w(adpcm,0,0x40);		/* Stop playing music */
		}
	}

}


/* Sound Routines */

static READ16_HANDLER( snowbros_68000_sound_r )
{
	return soundlatch_r(space,offset);
}


static WRITE16_HANDLER( snowbros_68000_sound_w )
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w(space, offset, data & 0xff);
		cputag_set_input_line(space->machine, "soundcpu", INPUT_LINE_NMI, PULSE_LINE);
	}
}

static WRITE16_HANDLER( semicom_soundcmd_w )
{
	if (ACCESSING_BITS_0_7) soundlatch_w(space,0,data & 0xff);
}

/* Snow Bros Memory Map */

static ADDRESS_MAP_START( snowbros_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
	AM_RANGE(0x200000, 0x200001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x300000, 0x300001) AM_READWRITE(snowbros_68000_sound_r,snowbros_68000_sound_w)
	AM_RANGE(0x400000, 0x400001) AM_WRITE(snowbros_flipscreen_w)
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("DSW1")
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("DSW2")
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x600000, 0x6001ff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x700000, 0x701fff) AM_DEVREADWRITE("pandora", pandora_spriteram_LSB_r, pandora_spriteram_LSB_w)
	AM_RANGE(0x800000, 0x800001) AM_WRITE(snowbros_irq4_ack_w)	/* IRQ 4 acknowledge */
	AM_RANGE(0x900000, 0x900001) AM_WRITE(snowbros_irq3_ack_w)	/* IRQ 3 acknowledge */
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(snowbros_irq2_ack_w)	/* IRQ 2 acknowledge */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x02, 0x03) AM_DEVREADWRITE("ymsnd", ym3812_r, ym3812_w)
	AM_RANGE(0x04, 0x04) AM_READWRITE(soundlatch_r, soundlatch_w)	/* goes back to the main CPU, checked during boot */
ADDRESS_MAP_END



static READ8_HANDLER( prot_io_r )
{
	// never read?
	return 0x00;
}

static UINT8 semicom_prot_offset = 0x00;

// probably not endian safe
static WRITE8_HANDLER( prot_io_w )
{
	switch (offset)
	{
		case 0x00:
		{
			UINT16 word = hyperpac_ram[(0xe000/2)+semicom_prot_offset];
			word = (word & 0xff00) | (data << 0);
			hyperpac_ram[(0xe000/2)+semicom_prot_offset] = word;
			break;
		}

		case 0x01:
		{
			UINT16 word = hyperpac_ram[(0xe000/2)+semicom_prot_offset];
			word = (word & 0x00ff) | (data << 8);
			hyperpac_ram[(0xe000/2)+semicom_prot_offset] = word;
			break;
		}

		case 0x02: // offset
		{
			semicom_prot_offset = data;
			break;
		}

		case 0x03: // ??
		{
			//logerror("offset %02x data %02x\n",offset,data);
			break;
		}
	}
}

/* Semicom AT89C52 MCU */
static ADDRESS_MAP_START( protection_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( protection_iomap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READWRITE(prot_io_r,prot_io_w)
ADDRESS_MAP_END

/* Winter Bobble - bootleg GFX chip */

static ADDRESS_MAP_START( wintbob_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
	AM_RANGE(0x200000, 0x200001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x300000, 0x300001) AM_READWRITE(snowbros_68000_sound_r,snowbros_68000_sound_w)
	AM_RANGE(0x400000, 0x400001) AM_WRITE(snowbros_flipscreen_w)
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("DSW1")
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("DSW2")
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x600000, 0x6001ff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x700000, 0x701fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x800000, 0x800001) AM_WRITE(snowbros_irq4_ack_w)	/* IRQ 4 acknowledge */
	AM_RANGE(0x900000, 0x900001) AM_WRITE(snowbros_irq3_ack_w)	/* IRQ 3 acknowledge */
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(snowbros_irq2_ack_w)	/* IRQ 2 acknowledge */
ADDRESS_MAP_END

/* Honey Dolls */

static ADDRESS_MAP_START( honeydol_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM AM_BASE(&hyperpac_ram)
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP	/* ? */
	AM_RANGE(0x300000, 0x300001) AM_WRITE(snowbros_68000_sound_w)	/* ? */
	AM_RANGE(0x400000, 0x400001) AM_WRITE(snowbros_irq4_ack_w)	/* IRQ 4 acknowledge */
	AM_RANGE(0x500000, 0x500001) AM_WRITE(snowbros_irq3_ack_w)	/* IRQ 3 acknowledge */
	AM_RANGE(0x600000, 0x600001) AM_WRITE(snowbros_irq2_ack_w)	/* IRQ 2 acknowledge */
	AM_RANGE(0x800000, 0x800001) AM_WRITENOP	/* ? */
	AM_RANGE(0x900000, 0x900001) AM_READ_PORT("DSW1")
	AM_RANGE(0x900002, 0x900003) AM_READ_PORT("DSW2")
	AM_RANGE(0x900004, 0x900005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xa00000, 0xa007ff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xb00000, 0xb01fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( honeydol_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xe010, 0xe010) AM_DEVREADWRITE("oki", okim6295_r,okim6295_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( honeydol_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x02, 0x03) AM_DEVREADWRITE("ymsnd", ym3812_r, ym3812_w)								// not connected?
	AM_RANGE(0x04, 0x04) AM_READWRITE(soundlatch_r, soundlatch_w)	/* goes back to the main CPU, checked during boot */
ADDRESS_MAP_END

/* Twin Adventure */

static WRITE16_HANDLER( twinadv_68000_sound_w )
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w(space, offset, data & 0xff);
		cputag_set_input_line(space->machine, "soundcpu", INPUT_LINE_NMI, PULSE_LINE);
	}
}

static ADDRESS_MAP_START( twinadv_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x200000, 0x200001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x300000, 0x300001) AM_READWRITE(snowbros_68000_sound_r,twinadv_68000_sound_w)
	AM_RANGE(0x400000, 0x400001) AM_WRITE(snowbros_flipscreen_w)

	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("DSW1")
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("DSW2")
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x600000, 0x6001ff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x700000, 0x701fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x800000, 0x800001) AM_WRITE(snowbros_irq4_ack_w)	/* IRQ 4 acknowledge */
	AM_RANGE(0x900000, 0x900001) AM_WRITE(snowbros_irq3_ack_w)	/* IRQ 3 acknowledge */
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(snowbros_irq2_ack_w)	/* IRQ 2 acknowledge */
ADDRESS_MAP_END

static WRITE8_DEVICE_HANDLER( twinadv_oki_bank_w )
{
	int bank = (data &0x02)>>1;

	if (data&0xfd) logerror ("Unused bank bits! %02x\n",data);

	downcast<okim6295_device *>(device)->set_bank_base(bank * 0x40000);
}

static ADDRESS_MAP_START( twinadv_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x02, 0x02) AM_READWRITE(soundlatch_r, soundlatch_w) // back to 68k?
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("oki", twinadv_oki_bank_w) // oki bank?
	AM_RANGE(0x06, 0x06) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)
ADDRESS_MAP_END


/* SemiCom Memory Map

the SemiCom games have slightly more ram and are protected
sound hardware is also different

*/

static ADDRESS_MAP_START( hyperpac_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM AM_BASE(&hyperpac_ram)
	AM_RANGE(0x300000, 0x300001) AM_WRITE(semicom_soundcmd_w)
//  AM_RANGE(0x400000, 0x400001) ???
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("DSW1")
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("DSW2")
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("SYSTEM")

	AM_RANGE(0x600000, 0x6001ff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x700000, 0x701fff) AM_DEVREADWRITE("pandora", pandora_spriteram_LSB_r,pandora_spriteram_LSB_w)
	AM_RANGE(0x800000, 0x800001) AM_WRITE(snowbros_irq4_ack_w)	/* IRQ 4 acknowledge */
	AM_RANGE(0x900000, 0x900001) AM_WRITE(snowbros_irq3_ack_w)	/* IRQ 3 acknowledge */
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(snowbros_irq2_ack_w)	/* IRQ 2 acknowledge */
ADDRESS_MAP_END

static ADDRESS_MAP_START( hyperpac_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xcfff) AM_ROM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM
	AM_RANGE(0xf000, 0xf001) AM_DEVREADWRITE("ymsnd", ym2151_r,ym2151_w)
	AM_RANGE(0xf002, 0xf002) AM_DEVREADWRITE("oki",okim6295_r,okim6295_w)
	AM_RANGE(0xf008, 0xf008) AM_READ(soundlatch_r)
ADDRESS_MAP_END

/* Same volume used for all samples at the Moment, could be right, we have no
   way of knowing .. */
static READ16_HANDLER( sb3_sound_r )
{
	return 0x0003;
}

static void sb3_play_music(running_machine *machine, int data)
{
	UINT8 *snd;

	/* sample is actually played in interrupt function so it loops */
	sb3_music = data;

	switch (data)
	{
		case 0x23:
		case 0x26:
		snd = memory_region(machine, "oki");
		memcpy(snd+0x20000, snd+0x80000+0x00000, 0x20000);
		sb3_music_is_playing = 1;
		break;

		case 0x24:
		snd = memory_region(machine, "oki");
		memcpy(snd+0x20000, snd+0x80000+0x20000, 0x20000);
		sb3_music_is_playing = 1;
		break;

		case 0x25:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		snd = memory_region(machine, "oki");
		memcpy(snd+0x20000, snd+0x80000+0x40000, 0x20000);
		sb3_music_is_playing = 1;
		break;

		case 0x2e:
		sb3_music_is_playing = 0;
		break;
	}
}

static void sb3_play_sound (running_device *device, int data)
{
	int status = okim6295_r(device,0);

	if ((status&0x01)==0x00)
	{
		okim6295_w(device,0,0x80|data);
		okim6295_w(device,0,0x00|0x12);
	}
	else if ((status&0x02)==0x00)
	{
		okim6295_w(device,0,0x80|data);
		okim6295_w(device,0,0x00|0x22);
	}
	else if ((status&0x04)==0x00)
	{
		okim6295_w(device,0,0x80|data);
		okim6295_w(device,0,0x00|0x42);
	}


}

static WRITE16_DEVICE_HANDLER( sb3_sound_w )
{
	if (data == 0x00fe)
	{
		sb3_music_is_playing = 0;
		okim6295_w(device,0,0x78);		/* Stop sounds */
	}
	else /* the alternating 0x00-0x2f or 0x30-0x5f might be something to do with the channels */
	{
		data = data>>8;

		if (data <= 0x21)
		{
			sb3_play_sound(device, data);
		}

		if (data>=0x22 && data<=0x31)
		{
			sb3_play_music(device->machine, data);
		}

		if ((data>=0x30) && (data<=0x51))
		{
			sb3_play_sound(device, data-0x30);
		}

		if (data>=0x52 && data<=0x5f)
		{
			sb3_play_music(device->machine, data-0x30);
		}

	}
}



static ADDRESS_MAP_START( snowbros3_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x03ffff) AM_ROM
	AM_RANGE( 0x100000, 0x103fff) AM_RAM
	AM_RANGE( 0x200000, 0x200001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE( 0x300000, 0x300001) AM_READ(sb3_sound_r) // ?
	AM_RANGE( 0x300000, 0x300001) AM_DEVWRITE("oki", sb3_sound_w)  // ?
	AM_RANGE( 0x400000, 0x400001) AM_WRITE(snowbros_flipscreen_w)
	AM_RANGE( 0x500000, 0x500001) AM_READ_PORT("DSW1")
	AM_RANGE( 0x500002, 0x500003) AM_READ_PORT("DSW2")
	AM_RANGE( 0x500004, 0x500005) AM_READ_PORT("SYSTEM")
	AM_RANGE( 0x600000, 0x6003ff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC (paletteram)
	AM_RANGE( 0x700000, 0x7021ff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x800000, 0x800001) AM_WRITE(snowbros_irq4_ack_w)	/* IRQ 4 acknowledge */
	AM_RANGE(0x900000, 0x900001) AM_WRITE(snowbros_irq3_ack_w)	/* IRQ 3 acknowledge */
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(snowbros_irq2_ack_w)	/* IRQ 2 acknowledge */
ADDRESS_MAP_END

/* Final Tetris */

static ADDRESS_MAP_START( finalttr_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM AM_BASE(&hyperpac_ram)
	AM_RANGE(0x300000, 0x300001) AM_WRITE(semicom_soundcmd_w)
//  AM_RANGE(0x400000, 0x400001) ???

	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("DSW1")
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("DSW2")
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("SYSTEM")

	AM_RANGE(0x600000, 0x6001ff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x700000, 0x701fff) AM_DEVREADWRITE("pandora", pandora_spriteram_LSB_r, pandora_spriteram_LSB_w)
	AM_RANGE(0x800000, 0x800001) AM_WRITE(snowbros_irq4_ack_w)	/* IRQ 4 acknowledge */
	AM_RANGE(0x900000, 0x900001) AM_WRITE(snowbros_irq3_ack_w)	/* IRQ 3 acknowledge */
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(snowbros_irq2_ack_w)	/* IRQ 2 acknowledge */
ADDRESS_MAP_END

static INPUT_PORTS_START( snowbros )
	PORT_START("DSW1")	/* 500001 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Region ) )		PORT_DIPLOCATION("SW1:1") /* Listed as "NOT USE" in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x01, "America (Romstar license)" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )	PORT_CONDITION("DSW1", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Must be low or game stops! */
													/* probably VBlank */

	PORT_START("DSW2")	/* 500003 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, "100k and every 200k" )
	PORT_DIPSETTING(    0x0c, "100k Only" )
	PORT_DIPSETTING(    0x08, "200k Only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability" )		PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:8") /* Listed as "NOT USE" in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")	/* 500005 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( snowbroj )
	PORT_INCLUDE(snowbros)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SW1:1") /* Listed as "NOT USE" in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( honeydol )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Show Girls" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Level_Select ) ) /* Up & Down to set level, then punch to start */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Slide Show On Boot-up" )  /* Joystick to scroll. Seems to happen once at boot up, then the game auto starts */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_VBLANK )	/* Must be low or game stops! */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Timer Speed" )     /* Based on Normal Difficulty, Stage 1-1 first try */
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) ) /* About 40 Seconds */
	PORT_DIPSETTING(      0x0008, "Fast" )            /* About 35 Seconds */
	PORT_DIPSETTING(      0x0004, "Faster" )          /* About 30 Seconds */
	PORT_DIPSETTING(      0x0000, "Fastest" )         /* About 20 Seconds */
	PORT_DIPNAME( 0x0030, 0x0020, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0030, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Max Vs Round" )
	PORT_DIPSETTING(      0x0080, "3" ) /* 44 Seconds each */
	PORT_DIPSETTING(      0x0000, "1" ) /* 89 Seconds */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( twinadv )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Level_Select ) ) /* P1 Button 2 to advance, P1 Button 1 to start, starts game with 10 credits */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_VBLANK )	/* Must be low or game stops! */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0008, 0x0008, "Ticket Mode #1" ) /* Shows on title screen "EVERY 4 GAMES = 1 TICKET" same as 0x0040 below? */
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Ticket Mode #2" ) /* Shows on title screen "EVERY 4 GAMES = 1 TICKET" same as 0x0008 above? */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x80,   0x0080, DEF_STR( Free_Play ) ) /* Always shows 24 credits */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( 4in1boot )
	PORT_START("DSW1")	/* 500001 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x78, 0x78, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SW1:4,5,6,7")
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x58, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x78, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x48, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x68, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Must be low or game stops! */
													/* probably VBlank */

	PORT_START("DSW2")	/* 500003 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")	/* 500005 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( hyperpac )
	PORT_START("DSW1")	/* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Lives ) )	PORT_DIPLOCATION("SW1:2")	// "Language" in the "test mode"
	PORT_DIPSETTING(      0x0002, "3" )							// "Korean"
	PORT_DIPSETTING(      0x0000, "5" )							// "English"
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coinage ) )	PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hardest ) )			// DEF_STR( Very_Hard )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)	// jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)	// fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)	// test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")	/* 500002.w */
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0000, "SW2:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)	// jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)	// fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)	// test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")	/* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( cookbib2 )
	PORT_START("DSW1")	/* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Max Vs Round" )		PORT_DIPLOCATION("SW1:2")	// "Language" in the "test mode"
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coinage ) )	PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hardest ) )			// DEF_STR( Very_Hard )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)	// jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)	// fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)	// test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")	/* 500002.w */
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0000, "SW2:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)	// jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)	// fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)	// test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")	/* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( cookbib3 )
	PORT_START("DSW1")	/* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ) )	PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x0020, "Level 1" )
	PORT_DIPSETTING(      0x0010, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x0070, "Level 4" )
	PORT_DIPSETTING(      0x0060, "Level 5" )
	PORT_DIPSETTING(      0x0050, "Level 6" )
	PORT_DIPSETTING(      0x0040, "Level 7" )
	PORT_DIPSETTING(      0x0030, "Level 8" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)	// jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)	// fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)	// test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")	/* 500002.w */
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )	PORT_DIPLOCATION("SW2:8") /* Will go into negative credits and cause graphics issues */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)	// jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)	// fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)	// test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")	/* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( moremore )
	PORT_START("DSW1")	/* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ) )	PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x0020, "Level 1" )
	PORT_DIPSETTING(      0x0010, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x0070, "Level 4" )
	PORT_DIPSETTING(      0x0060, "Level 5" )
	PORT_DIPSETTING(      0x0050, "Level 6" )
	PORT_DIPSETTING(      0x0040, "Level 7" )
	PORT_DIPSETTING(      0x0030, "Level 8" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)	// jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)	// fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)	// test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")	/* 500002.w */
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)	// jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)	// fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)	// test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")	/* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

// don't trust the test mode! <-- Verified via actual game play: Demo Sounds, Coinage & Free Play
static INPUT_PORTS_START( pzlbreak )
	PORT_START("DSW1")	/* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ) )	PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x0020, "Level 1" )
	PORT_DIPSETTING(      0x0010, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x0070, "Level 4" )
	PORT_DIPSETTING(      0x0060, "Level 5" )
	PORT_DIPSETTING(      0x0050, "Level 6" )
	PORT_DIPSETTING(      0x0040, "Level 7" )
	PORT_DIPSETTING(      0x0030, "Level 8" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")	/* 500002.w */
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")	/* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( toppyrap )
	PORT_START("DSW1")	/* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW1:2" )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coinage ) )	PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)	// jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)	// fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)	// test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")	/* 500002.w */
	PORT_DIPNAME( 0x03,   0x0003, DEF_STR( Lives ) )	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, "Time" )			PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0004, "40 Seconds" )
	PORT_DIPSETTING(      0x0008, "50 Seconds" )
	PORT_DIPSETTING(      0x000c, "60 Seconds" )
	PORT_DIPSETTING(      0x0000, "70 Seconds" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPNAME( 0x0020, 0x0020, "God Mode" )		PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Internal Test" )		PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)	// jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)	// fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)	// test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")	/* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( finalttr )
	PORT_START("DSW1")	/* 500001 */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
/*  PORT_DIPSETTING(      0x0018, DEF_STR( 1C_1C ) ) Duplicate setting? */
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Must be low or game stops! */
													/* probably VBlank */
	PORT_START("DSW2")	/* 500003 */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Time" )
	PORT_DIPSETTING(      0x0000, "60 Seconds" )
	PORT_DIPSETTING(      0x000c, "90 Seconds" )
	PORT_DIPSETTING(      0x0008, "120 Seconds" )
	PORT_DIPSETTING(      0x0004, "150 Seconds" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")	/* 500005 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* SnowBros */

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ STEP8(0,4), STEP8(8*32,4) },
	{ STEP8(0,32), STEP8(16*32,32) },
	32*32
};

static GFXDECODE_START( snowbros )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,  0, 16 )
GFXDECODE_END

/* Honey Doll */

static const gfx_layout honeydol_tilelayout8bpp =
{
	16,16,
	RGN_FRAC(1,2),
	8,
	{ 0, 1, 2, 3, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3},
	{ STEP8(0,4), STEP8(8*32,4) },
	{ STEP8(0,32), STEP8(16*32,32) },
	32*32
};

static GFXDECODE_START( honeydol )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,  0, 64 ) // how does it use 0-15
	GFXDECODE_ENTRY( "gfx2", 0, honeydol_tilelayout8bpp,  0, 4 )

GFXDECODE_END

static GFXDECODE_START( twinadv )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,  0, 64 )
GFXDECODE_END

/* Winter Bobble */

static const gfx_layout tilelayout_wb =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ STEP4(3*4,-4), STEP4(7*4,-4), STEP4(11*4,-4), STEP4(15*4,-4) },
	{ STEP16(0,64) },
	16*64
};

static GFXDECODE_START( wb )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout_wb,  0, 16 )
GFXDECODE_END

/* SemiCom */

static const gfx_layout hyperpac_tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 8*32+4, 8*32+0, 20,16, 8*32+20, 8*32+16,
	  12, 8, 8*32+12, 8*32+8, 28, 24, 8*32+28, 8*32+24 },
	{ 0*32, 2*32, 1*32, 3*32, 16*32+0*32, 16*32+2*32, 16*32+1*32, 16*32+3*32,
	  4*32, 6*32, 5*32, 7*32, 16*32+4*32, 16*32+6*32, 16*32+5*32, 16*32+7*32 },
	32*32
};


static const gfx_layout sb3_tilebglayout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{8, 9,10, 11, 0, 1, 2, 3  },
	{ 0, 4, 16, 20, 32, 36, 48, 52,
	512+0,512+4,512+16,512+20,512+32,512+36,512+48,512+52},
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
	1024+0*16,1024+1*64,1024+2*64,1024+3*64,1024+4*64,1024+5*64,1024+6*64,1024+7*64},
	32*64
};


static GFXDECODE_START( sb3 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, sb3_tilebglayout,  0, 2 )
GFXDECODE_END

static GFXDECODE_START( hyperpac )
	GFXDECODE_ENTRY( "gfx1", 0, hyperpac_tilelayout,  0, 16 )
GFXDECODE_END

/* handler called by the 3812/2151 emulator when the internal timers cause an IRQ */
static void irqhandler(running_device *device, int irq)
{
	cputag_set_input_line(device->machine, "soundcpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

/* SnowBros Sound */

static const ym3812_interface ym3812_config =
{
	irqhandler
};

/* SemiCom Sound */

static const ym2151_interface ym2151_config =
{
	irqhandler
};


static MACHINE_RESET (semiprot)
{
	UINT16 *PROTDATA = (UINT16*)memory_region(machine, "user1");
	int i;

	for (i = 0;i < 0x200/2;i++)
	hyperpac_ram[0xf000/2 + i] = PROTDATA[i];
}

static MACHINE_RESET (finalttr)
{
	UINT16 *PROTDATA = (UINT16*)memory_region(machine, "user1");
	int i;

	for (i = 0;i < 0x200/2;i++)
	hyperpac_ram[0x2000/2 + i] = PROTDATA[i];
}

static const kaneko_pandora_interface snowbros_pandora_config =
{
	"screen",	/* screen tag */
	0,	/* gfx_region */
	0, 0	/* x_offs, y_offs */
};

static MACHINE_DRIVER_START( snowbros )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 8000000) /* 8 Mhz - confirmed */
	MDRV_CPU_PROGRAM_MAP(snowbros_map)
	MDRV_CPU_VBLANK_INT_HACK(snowbros_interrupt,3)

	MDRV_CPU_ADD("soundcpu", Z80, 6000000) /* 6 MHz - confirmed */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(sound_io_map)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57.5) /* ~57.5 - confirmed */
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(snowbros)
	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_UPDATE(snowbros)
	MDRV_VIDEO_EOF(snowbros)

	MDRV_KANEKO_PANDORA_ADD("pandora", snowbros_pandora_config)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, 3000000)
	MDRV_SOUND_CONFIG(ym3812_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( wintbob )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(snowbros)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_CLOCK(10000000) /* 10mhz - Confirmed */
	MDRV_CPU_PROGRAM_MAP(wintbob_map)

	MDRV_DEVICE_REMOVE("pandora")

	/* video hardware */
	MDRV_GFXDECODE(wb)
	MDRV_VIDEO_UPDATE(wintbob)
	MDRV_VIDEO_EOF(0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( semicom )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(snowbros)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_CLOCK(16000000) /* 16mhz or 12mhz ? */
	MDRV_CPU_PROGRAM_MAP(hyperpac_map)

	MDRV_CPU_MODIFY("soundcpu")
	MDRV_CPU_CLOCK(4000000) /* 4.0 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(hyperpac_sound_map)

	MDRV_GFXDECODE(hyperpac)

	/* sound hardware */
	MDRV_SOUND_REPLACE("ymsnd", YM2151, 4000000)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "mono", 0.10)
	MDRV_SOUND_ROUTE(1, "mono", 0.10)

	MDRV_OKIM6295_ADD("oki", 999900, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



static MACHINE_DRIVER_START( semicom_mcu )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(semicom)

	MDRV_CPU_ADD("protection", I8052, 16000000)  // AT89C52
	MDRV_CPU_PROGRAM_MAP(protection_map)
	MDRV_CPU_IO_MAP(protection_iomap)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( semiprot )
	MDRV_IMPORT_FROM(semicom)
	MDRV_MACHINE_RESET ( semiprot )
MACHINE_DRIVER_END

/*

Honey Doll - Barko Corp 1995

Rom Board include a Cypress cy7C382-0JC chip

Main Board :

CPU : 1 X MC68000P12
      1 X Z80B

1 X Oki M6295
2 X Cypress CY7C384A-XJC

2 x quartz - 12Mhz and 16Mhz


See included pics
*/

static MACHINE_DRIVER_START( honeydol )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)
	MDRV_CPU_PROGRAM_MAP(honeydol_map)
	MDRV_CPU_VBLANK_INT_HACK(snowbros_interrupt,3)

	MDRV_CPU_ADD("soundcpu", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(honeydol_sound_map)
	MDRV_CPU_IO_MAP(honeydol_sound_io_map)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(honeydol)
	MDRV_PALETTE_LENGTH(0x800/2)

	MDRV_VIDEO_UPDATE(honeydol)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	/* sound hardware */

	MDRV_SOUND_ADD("ymsnd", YM3812, 3000000)
	MDRV_SOUND_CONFIG(ym3812_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)


	MDRV_OKIM6295_ADD("oki", 999900, OKIM6295_PIN7_HIGH) /* freq? */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( twinadv )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000) // or 12
	MDRV_CPU_PROGRAM_MAP(twinadv_map)
	MDRV_CPU_VBLANK_INT_HACK(snowbros_interrupt,3)

	MDRV_CPU_ADD("soundcpu", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(twinadv_sound_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(twinadv)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_UPDATE(twinadv)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	/* sound hardware */
	MDRV_OKIM6295_ADD("oki", 12000000/12, OKIM6295_PIN7_HIGH) /* freq? */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/*

Final Tetris

the pcb is korean and probably original even if it's very cheap.

osc 12mhz
osc 3.579545mhz
68000
z8004b
CA5101 (sound chip)
OKI M6295 (sound chip)
Intel P8752 (mcu)

2x dips banks

*/

static MACHINE_DRIVER_START( finalttr )
	MDRV_IMPORT_FROM(semicom)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_CLOCK(12000000)
	MDRV_CPU_PROGRAM_MAP(finalttr_map)

	MDRV_CPU_MODIFY("soundcpu")
	MDRV_CPU_CLOCK(3578545)

	MDRV_MACHINE_RESET ( finalttr )

	MDRV_SOUND_REPLACE("ymsnd", YM2151, 4000000)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "mono", 0.08)
	MDRV_SOUND_ROUTE(1, "mono", 0.08)

	MDRV_OKIM6295_REPLACE("oki", 999900, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( _4in1 )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(semicom)
	MDRV_GFXDECODE(snowbros)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( snowbro3 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000) /* 16mhz or 12mhz ? */
	MDRV_CPU_PROGRAM_MAP(snowbros3_map)
	MDRV_CPU_VBLANK_INT_HACK(snowbro3_interrupt,3)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(sb3)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_UPDATE(snowbro3)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", 999900, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( snowbros )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "sn6.bin",  0x00000, 0x20000, CRC(4899ddcf) SHA1(47d750d3022a80e47ffabe47566bb2556cc8d477) )
	ROM_LOAD16_BYTE( "sn5.bin",  0x00001, 0x20000, CRC(ad310d3f) SHA1(f39295b38d99087dbb9c5b00bf9cb963337a50e2) )

	ROM_REGION( 0x10000, "soundcpu", 0 )	/* 64k for z80 sound code */
	ROM_LOAD( "sbros-4.29",   0x0000, 0x8000, CRC(e6eab4e4) SHA1(d08187d03b21192e188784cb840a37a7bdb5ad32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sbros-1.41",   0x00000, 0x80000, CRC(16f06b3a) SHA1(c64d3b2d32f0f0fcf1d8c5f02f8589d59ddfd428) )
	/* where were these from, a bootleg? */
//  ROM_LOAD( "ch0",          0x00000, 0x20000, CRC(36d84dfe) SHA1(5d45a750220930bc409de30f19282bb143fbf94f) )
//  ROM_LOAD( "ch1",          0x20000, 0x20000, CRC(76347256) SHA1(48ec03965905adaba5e50eb3e42a2813f7883bb4) )
//  ROM_LOAD( "ch2",          0x40000, 0x20000, CRC(fdaa634c) SHA1(1271c74df7da7596caf67caae3c51b4c163a49f4) )
//  ROM_LOAD( "ch3",          0x60000, 0x20000, CRC(34024aef) SHA1(003a9b9ee3aaab3d787894d3d4126d372b19d2a8) )
ROM_END

ROM_START( snowbrosa )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "sbros-3a.5",  0x00000, 0x20000, CRC(10cb37e1) SHA1(786be4640f8df2c81a32decc189ea7657ace00c6) )
	ROM_LOAD16_BYTE( "sbros-2a.6",  0x00001, 0x20000, CRC(ab91cc1e) SHA1(8cff61539dc7d35fcbf110d3e54fc1883e7b8509) )

	ROM_REGION( 0x10000, "soundcpu", 0 )	/* 64k for z80 sound code */
	ROM_LOAD( "sbros-4.29",   0x0000, 0x8000, CRC(e6eab4e4) SHA1(d08187d03b21192e188784cb840a37a7bdb5ad32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sbros-1.41",   0x00000, 0x80000, CRC(16f06b3a) SHA1(c64d3b2d32f0f0fcf1d8c5f02f8589d59ddfd428) )
ROM_END

ROM_START( snowbrosb )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "sbros3-a",     0x00000, 0x20000, CRC(301627d6) SHA1(0d1dc70091c87e9c27916d4232ff31b7381a64e1) )
	ROM_LOAD16_BYTE( "sbros2-a",     0x00001, 0x20000, CRC(f6689f41) SHA1(e4fd27b930a31479c0d99e0ddd23d5db34044666) )

	ROM_REGION( 0x10000, "soundcpu", 0 )	/* 64k for z80 sound code */
	ROM_LOAD( "sbros-4.29",   0x0000, 0x8000, CRC(e6eab4e4) SHA1(d08187d03b21192e188784cb840a37a7bdb5ad32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sbros-1.41",   0x00000, 0x80000, CRC(16f06b3a) SHA1(c64d3b2d32f0f0fcf1d8c5f02f8589d59ddfd428) )
ROM_END

ROM_START( snowbrosc )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "3-a.ic5",  0x00000, 0x20000, CRC(e1bc346b) SHA1(a20c343d9ed2ad4f785d21076499008edad251f9) )
	ROM_LOAD16_BYTE( "2-a.ic6",  0x00001, 0x20000, CRC(1be27f9d) SHA1(76dd14480b9274831e51016f7bb57459d7b15cf9) )

	ROM_REGION( 0x10000, "soundcpu", 0 )	/* 64k for z80 sound code */
	ROM_LOAD( "sbros-4.29",   0x0000, 0x8000, CRC(e6eab4e4) SHA1(d08187d03b21192e188784cb840a37a7bdb5ad32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sbros-1.41",   0x00000, 0x80000, CRC(16f06b3a) SHA1(c64d3b2d32f0f0fcf1d8c5f02f8589d59ddfd428) )
ROM_END

ROM_START( snowbrosd ) /* Korean release, but no specific "For use in Korea only..." notice screen */
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "sbk_3-a.bin",   0x00000, 0x20000, CRC(97174d40) SHA1(481e8c680af8b03d4bcf97b87ca0ba5a3ffca0d7) )
	ROM_LOAD16_BYTE( "sbk_2-a.bin",   0x00001, 0x20000, CRC(80cc80e5) SHA1(1eeca0924c93e9f0536683160e80c59871569088) )

	ROM_REGION( 0x10000, "soundcpu", 0 )	/* 64k for z80 sound code */
	ROM_LOAD( "sbros-4.29",   0x0000, 0x8000, CRC(e6eab4e4) SHA1(d08187d03b21192e188784cb840a37a7bdb5ad32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sbros-1.41",   0x00000, 0x80000, CRC(16f06b3a) SHA1(c64d3b2d32f0f0fcf1d8c5f02f8589d59ddfd428) )
ROM_END

ROM_START( snowbrosj )/* "For use in Japan only..." notice screen */
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "snowbros.3",   0x00000, 0x20000, CRC(3f504f9e) SHA1(700758b114c3fde6ea8f84222af0850dba13cd3b) )
	ROM_LOAD16_BYTE( "snowbros.2",   0x00001, 0x20000, CRC(854b02bc) SHA1(4ad1548eef94dcb95119cb4a7dcdefa037591b5b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )	/* 64k for z80 sound code */
	ROM_LOAD( "sbros-4.29",   0x0000, 0x8000, CRC(e6eab4e4) SHA1(d08187d03b21192e188784cb840a37a7bdb5ad32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	/* The gfx ROM (snowbros.1) was bad, I'm using the ones from the other sets. */
	ROM_LOAD( "sbros-1.41",   0x00000, 0x80000, CRC(16f06b3a) SHA1(c64d3b2d32f0f0fcf1d8c5f02f8589d59ddfd428) )
ROM_END

ROM_START( wintbob )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "wb3", 0x00000, 0x10000, CRC(b9719767) SHA1(431c97d409f2a5ff7f46116a4d8907e446434431) )
	ROM_LOAD16_BYTE( "wb1", 0x00001, 0x10000, CRC(a4488998) SHA1(4e927e31c1b865dbdba2b985c7a819a07e2e81b8) )

	/* The wb03.bin below is bad, the set has a different copyright message (IN KOREA is replaced with 1990)
       but also clearly suffers from bitrot at the following addresses
        4FC2, 5F02, 6642, D6C2, D742
       in all cases bit 0x20 is incorrectly set in the bad rom
    */

//  ROM_LOAD16_BYTE( "wb03.bin", 0x00000, 0x10000, CRC(df56e168) SHA1(20dbabdd97e6f3d4bf6500bf9e8476942cb48ae3) )
//  ROM_LOAD16_BYTE( "wb01.bin", 0x00001, 0x10000, CRC(05722f17) SHA1(9356e2488ea35e0a2978689f2ca6dfa0d57fd2ed) )

	ROM_LOAD16_BYTE( "wb04.bin", 0x20000, 0x10000, CRC(53be758d) SHA1(56cf85ba23fe699031d73e8f367a1b8ac837d5f8) )
	ROM_LOAD16_BYTE( "wb02.bin", 0x20001, 0x10000, CRC(fc8e292e) SHA1(857cfeb0be121e64e6117120514ae1f2ffeae4d6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )	/* 64k for z80 sound code */
	ROM_LOAD( "wb05.bin",     0x0000, 0x10000, CRC(53fe59df) SHA1(a99053e82b9fed76f744fa9f67078294641c6317) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	/* probably identical data to Snow Bros, in a different format */
	ROM_LOAD16_BYTE( "wb13.bin",     0x00000, 0x10000, CRC(426921de) SHA1(5107c58e7e08d71895baa67fe260b17ebd61389c) )
	ROM_LOAD16_BYTE( "wb06.bin",     0x00001, 0x10000, CRC(68204937) SHA1(fd2ef93df5fd8aa2d36072858dbcfce41157ef3e) )
	ROM_LOAD16_BYTE( "wb12.bin",     0x20000, 0x10000, CRC(ef4e04c7) SHA1(17158b61b3c158e0491db9abb2e1a8c20d981d37) )
	ROM_LOAD16_BYTE( "wb07.bin",     0x20001, 0x10000, CRC(53f40978) SHA1(058bbf3b7877f0cd320383e0386c5959e0d6589b) )
	ROM_LOAD16_BYTE( "wb11.bin",     0x40000, 0x10000, CRC(41cb4563) SHA1(94f1d12d299ac08fc8522139e1927f0cf739be75) )
	ROM_LOAD16_BYTE( "wb08.bin",     0x40001, 0x10000, CRC(9497b88c) SHA1(367c6106276f3816528341f11f3a97ae458d25cd) )
	ROM_LOAD16_BYTE( "wb10.bin",     0x60000, 0x10000, CRC(5fa22b1e) SHA1(1164003d873e9738a3ca133cce689c7120061e3c) )
	ROM_LOAD16_BYTE( "wb09.bin",     0x60001, 0x10000, CRC(9be718ca) SHA1(5c195e4f13efbdb229201d2408d018861bf389cc) )
ROM_END

/* Barko */

ROM_START( honeydol )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "d-16.uh12",  0x00001, 0x20000, CRC(cee1a2e3) SHA1(6d1ff5358ec704616b724eea2ab9b60b84709eb1) )
	ROM_LOAD16_BYTE( "d-17.ui12",  0x00000, 0x20000, CRC(cac44154) SHA1(2c30dc033001fc9303da7e117e3401bc7af16607) )

	ROM_REGION( 0x10000, "soundcpu", 0 )	/* 64k for z80 sound code */
	ROM_LOAD( "d-12.uh15",   0x0000, 0x8000, CRC(386f1b63) SHA1(d719875226cd3d380e2ebec49209590d91b6f07b) )

	ROM_REGION( 0x80000, "gfx1", 0 ) // 4 bpp gfx
	ROM_LOAD( "d-13.1",          0x000000, 0x80000, CRC(ff6a57fb) SHA1(2fbf61f4ac2655a60b1fa82bb6d001f0ef8b4654) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "d-14.5",          0x000000, 0x80000, CRC(2178996f) SHA1(04368384cb191b28b23199c8175e93271ab79103) )
	ROM_LOAD( "d-15.6",          0x080000, 0x80000, CRC(6629239e) SHA1(5f462c04eb11c2b662236fd65bbf74fa08038eec) )
	ROM_LOAD( "d-18.9",          0x100000, 0x80000, CRC(0210507a) SHA1(5b7348bf253b1ae8bfa86cdee2ff80aa2b3faa79) )
	ROM_LOAD( "d-19.10",         0x180000, 0x80000, CRC(d27375d5) SHA1(2a39ce9b985e00a290c3ea75be3b1edbc00d39ec) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "d-11.u14", 0x00000, 0x40000, CRC(f3ee4861) SHA1(f24f1f855ae6c96a6d84a4b5e5c196df8f922d0a) )
ROM_END

/*

+--------------------------------+
|      Z80A                      |
|GL324  6116                  ua4|
|M6295 uh15  HY6264           ua5|
|J     sra   HY6264           ua6|
|A     srb   84256               |
|M           84256               |
|M           84256    CY7C384A   |
|A           84256               |
| SWA1       uh12                |
|      68000 84256               |
| SWA2       ui12  12MHz 16MHz   |
+--------------------------------+

Produttore: Barko
N.revisione: S16K951102
CPU
1x 68000 (main)
1x Z8400B (sound)
1x OKI M6295 (sound)
1x GL324 (sound)
1x CY7C384A
1x oscillator 12.000MHz
1x oscillator 16.000MHz

ROMs

1x 27256 (uh15)
2x M27C2001 (sra,srb)
1x AM27C010 (12)
1x D27C010 (13)
1x M27C4001 (14)
1x AT27C040 (15)
1x TMS27C040 (16)

6x HY18CV8S (read protected)

*/

ROM_START( twinadv )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "13.uh12",  0x00001, 0x20000, CRC(9f70a39b) SHA1(d49823be58b00c4c5a4f6cc4e4371531492aff1e) )
	ROM_LOAD16_BYTE( "12.ui12",  0x00000, 0x20000, CRC(d8776495) SHA1(15b93ded80bf9f240faef2d89b6076f33f1f4ece) )

	ROM_REGION( 0x10000, "soundcpu", 0 )	/* 64k for z80 sound code */
	ROM_LOAD( "uh15.bin", 0x0000, 0x8000, CRC(3d5acd08) SHA1(c19f686862dfc12d2fa91c2dd3d3b75d9cb410c3) )

	ROM_REGION( 0x180000, "gfx1", 0 ) /* 4bpp gfx */
	ROM_LOAD( "16.ua4", 0x000000, 0x80000, CRC(f491e171) SHA1(f31b945b0c4b30d1b3dc6c5928b77aad4e956bc7) )
	ROM_LOAD( "15.ua5", 0x080000, 0x80000, CRC(79a08b8d) SHA1(034c0a3b9e27ac174092d265b32fb82d6ee45d47) )
	ROM_LOAD( "14.ua6", 0x100000, 0x80000, CRC(79faee0b) SHA1(7421a5fa038d01658ba5ac1f65ea87b97ac25c36) )

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples - both banks are almost the same */
	/* todo, check bank ordering .. */
	ROM_LOAD( "sra.bin", 0x00000, 0x40000, CRC(82f452c4) SHA1(95ad6ede87ceafb045ed7df40496baf96190b97f) ) // bank 1
	ROM_LOAD( "srb.bin", 0x40000, 0x40000, CRC(109e51e6) SHA1(3344c68d63bbad4a02b47143b2d2f72ce9bcb4bb) ) // bank 2
ROM_END

ROM_START( twinadvk )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "uh12",  0x00001, 0x20000, CRC(e0bcc738) SHA1(7fc6a793fcdd80122c0ac6409ae4cac5597b7b5a) )
	ROM_LOAD16_BYTE( "ui12",  0x00000, 0x20000, CRC(a3ee6451) SHA1(9c0b415a2f325513739f2047780c2a56df350aa5) )

	ROM_REGION( 0x10000, "soundcpu", 0 )	/* 64k for z80 sound code */
	ROM_LOAD( "uh15.bin", 0x0000, 0x8000, CRC(3d5acd08) SHA1(c19f686862dfc12d2fa91c2dd3d3b75d9cb410c3) )

	ROM_REGION( 0x180000, "gfx1", 0 ) /* 4bpp gfx */
	ROM_LOAD( "ua4", 0x000000, 0x80000, CRC(a5aff49b) SHA1(ee162281ba643729ee007f9634c21fadd3c1cb48) )
	ROM_LOAD( "ua5", 0x080000, 0x80000, CRC(f83b3b97) SHA1(2e967d49ef411d164a0b6cf32444f60fcd8068a9) )
	ROM_LOAD( "ua6", 0x100000, 0x80000, CRC(4dfcffb9) SHA1(c157e031acbb321b9435389f9fc4e1ffebca106d) )

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples - both banks are almost the same */
	/* todo, check bank ordering .. */
	ROM_LOAD( "sra.bin", 0x00000, 0x40000, CRC(82f452c4) SHA1(95ad6ede87ceafb045ed7df40496baf96190b97f) ) // bank 1
	ROM_LOAD( "srb.bin", 0x40000, 0x40000, CRC(109e51e6) SHA1(3344c68d63bbad4a02b47143b2d2f72ce9bcb4bb) ) // bank 2
ROM_END


/* SemiCom Games */

ROM_START( hyperpac )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hyperpac.h12", 0x00001, 0x20000, CRC(2cf0531a) SHA1(c4321d728845035507352d0bcf4348d28b92e85e) )
	ROM_LOAD16_BYTE( "hyperpac.i12", 0x00000, 0x20000, CRC(9c7d85b8) SHA1(432d5fbe8bef875ce4a9aeb74a7b57dc79c709fd) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "hyperpac.u1", 0x00000, 0x10000 , CRC(03faf88e) SHA1(a8da883d4b765b809452bbffca37ff224edbe86d) )

	ROM_REGION( 0x10000, "protection", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "at89c52.bin", 0x00000, 0x2000 , CRC(291f9326) SHA1(e440ce7d92188faa86e02e7f9db4ec6bce21efd3) ) /* decapped */

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "hyperpac.j15", 0x00000, 0x40000, CRC(fb9f468d) SHA1(52857b1a04c64ac853340ebb8e92d98eabea8bc1) )

	ROM_REGION( 0x0c0000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "hyperpac.a4", 0x000000, 0x40000, CRC(bd8673da) SHA1(8466355894da4d2c9a54d03a833cc9b4ec0c67eb) )
	ROM_LOAD( "hyperpac.a5", 0x040000, 0x40000, CRC(5d90cd82) SHA1(56be68478a81bb4e1011990da83334929a0ac886) )
	ROM_LOAD( "hyperpac.a6", 0x080000, 0x40000, CRC(61d86e63) SHA1(974c634607993924fa098eff106b1b288bec1e26) )
ROM_END

ROM_START( hyperpacb )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hpacuh12.bin", 0x00001, 0x20000, CRC(633ab2c6) SHA1(534435fa602adebf651e1d42f7c96b01eb6634ef) )
	ROM_LOAD16_BYTE( "hpacui12.bin", 0x00000, 0x20000, CRC(23dc00d1) SHA1(8d4d00f450b94912adcbb24073f9b3b01eab0450) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "hyperpac.u1", 0x00000, 0x10000 , CRC(03faf88e) SHA1(a8da883d4b765b809452bbffca37ff224edbe86d) ) // was missing from this set, using the one from the original

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "hyperpac.j15", 0x00000, 0x40000, CRC(fb9f468d) SHA1(52857b1a04c64ac853340ebb8e92d98eabea8bc1) )

	ROM_REGION( 0x0c0000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "hyperpac.a4", 0x000000, 0x40000, CRC(bd8673da) SHA1(8466355894da4d2c9a54d03a833cc9b4ec0c67eb) )
	ROM_LOAD( "hyperpac.a5", 0x040000, 0x40000, CRC(5d90cd82) SHA1(56be68478a81bb4e1011990da83334929a0ac886) )
	ROM_LOAD( "hyperpac.a6", 0x080000, 0x40000, CRC(61d86e63) SHA1(974c634607993924fa098eff106b1b288bec1e26) )
ROM_END

ROM_START( twinkle )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "uh12.bin", 0x00001, 0x20000, CRC(a99626fe) SHA1(489098a2ceb36df97b6b1d59b7b696300deee3ab) )
	ROM_LOAD16_BYTE( "ui12.bin", 0x00000, 0x20000, CRC(5af73684) SHA1(9be43e5c71152d515366e422eb077a41dbb3fe62) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u1.bin", 0x00000, 0x10000 , CRC(e40481da) SHA1(1c1fabcb67693235eaa6ff59ae12a35854b5564a) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
       shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200, CRC(00d3e4b4) SHA1(afa359a8b48605ff034133bad2a0a182429dec71) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uj15.bin", 0x00000, 0x40000, CRC(0a534b37) SHA1(b7d780eb4668f1f757a60884c022f5bbc424dc97) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "ua4.bin", 0x000000, 0x80000, CRC(6b64bb09) SHA1(547eac1ad931a6b937dff0b922d06af92cc7ab73) )
ROM_END


ROM_START( pzlbreak )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "4.uh12", 0x00001, 0x20000, CRC(b3f04f80) SHA1(79b5414727004719ff172e084a672b21e955f0bc) )
	ROM_LOAD16_BYTE( "5.ui12", 0x00000, 0x20000, CRC(13c298a0) SHA1(9455de7ea45c9a61ed6105023eb909c086c44007) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "0.u1", 0x00000, 0x10000 , CRC(1ad646b7) SHA1(0132baa097e48df2450afdcd316375dc546ea4d0) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", ROMREGION_ERASEFF ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
       shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200, CRC(092cb794) SHA1(eb2b336d97b440453ca37ee7605654b35dfb6bad) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "1.uj15", 0x00000, 0x40000, CRC(dbfae77c) SHA1(cc509d52cd9c608fc80df799890e62e7b4c143c6) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "2.ua4", 0x000000, 0x80000, CRC(d211705a) SHA1(b3a7f8198dc8c034b17b843b2ab0298426de3f55) )
	ROM_LOAD( "3.ua5", 0x080000, 0x80000, CRC(6cdb73e9) SHA1(649e91ee54de2b359a207bed4d950db95515a3d8) )
ROM_END


ROM_START( toppyrap )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "uh12.bin", 0x00001, 0x40000, CRC(6f5ad699) SHA1(42f7201d6274ff8338a7d4627af99001f473e841) )
	ROM_LOAD16_BYTE( "ui12.bin", 0x00000, 0x40000, CRC(caf5a7e1) SHA1(b521b2f06a804a52dad1b07657db2a29e1411844) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u1.bin", 0x00000, 0x10000 , CRC(07f50947) SHA1(83740655ab5f677bd009191bb0de60e237aaa11c) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this contains the code for 2 of the IRQ functions, but the game only uses one of them, the other is
       executed from ROM.  The version in ROM is slightly patched version so maybe there is an earlier revision
       which uses the code provided by the MCU instead */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200, CRC(0704e6c7) SHA1(22387257db569990378c304af9677e6dc1436207) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uj15.bin", 0x00000, 0x20000, CRC(a3bacfd7) SHA1(d015d8bd26d0189fc13d09fefcb9b8baaaacec8a) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "ua4.bin", 0x000000, 0x80000, CRC(a9577bcf) SHA1(9918d982ebee1c88bd203fa2b3ce2468c160fb95) )
	ROM_LOAD( "ua5.bin", 0x080000, 0x80000, CRC(7179d32d) SHA1(dae7126401b5bb7f99689587e05a8bf5033ec06e) )
	ROM_LOAD( "ua6.bin", 0x100000, 0x80000, CRC(4834e5b1) SHA1(cd8a4c329b2bfe1a9c2dea9d72ca09b71366c60a) )
	ROM_LOAD( "ua7.bin", 0x180000, 0x80000, CRC(663dd099) SHA1(84b52af54ac49e8b4bae23995e3cf94494be2bb3) )
ROM_END

ROM_START( moremore )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u52.bin",  0x00001, 0x40000, CRC(cea4b246) SHA1(5febcb5dda6581caccfe9079b28c2366dfc1db2b) )
	ROM_LOAD16_BYTE( "u74.bin",  0x00000, 0x40000, CRC(2acdcb88) SHA1(74d661d07752bbccab7eab151209a414e9bf7675) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u35.bin", 0x00000, 0x10000 , CRC(92dc95fc) SHA1(f04e63cc680835458246989532faf5657e28db13) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
       shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(782dd2aa) SHA1(2587734271e0c85cb76bcdee171366c4e6fc9f81) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u14.bin", 0x00000, 0x40000, CRC(90580088) SHA1(c64de2c0db95ab4ce06fc0a29c0cc8b7f3deeb28) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u75.bin", 0x000000, 0x80000, CRC(d671815c) SHA1(a7e8d3bf688ce51b5d9a2b306cc557974328c322) )
	ROM_LOAD( "u76.bin", 0x080000, 0x80000, CRC(e0d479e8) SHA1(454b53949664aca07a86229d3b6c9ce4e9449ea6) )
	ROM_LOAD( "u77.bin", 0x100000, 0x80000, CRC(60a281da) SHA1(3f268f8b1cd8efd3e32d0fcdba5483c93122800e) )
	ROM_LOAD( "u78.bin", 0x180000, 0x80000, CRC(e2723b4e) SHA1(6b4ba1e2e937b3231d76526af3f5a4a67144e4d5) )
ROM_END

ROM_START( moremorp )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mmp_u52.bin",  0x00001, 0x40000, CRC(66baf9b2) SHA1(f1d383a94ef4313cb02c59ace17b9562eddcfb3c) )
	ROM_LOAD16_BYTE( "mmp_u74.bin",  0x00000, 0x40000, CRC(7c6fede5) SHA1(41bc539a6efe9eb2304243701857b972d2170bcf) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "mmp_u35.bin", 0x00000, 0x10000 , CRC(4d098cad) SHA1(a79d417e7525a25dd6697da9f3d1de269e759d2e) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
       shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(782dd2aa) SHA1(2587734271e0c85cb76bcdee171366c4e6fc9f81) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "mmp_u14.bin", 0x00000, 0x40000, CRC(211a2566) SHA1(48138547822a8e76c101dd4189d581f80eee1e24) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "mmp_u75.bin", 0x000000, 0x80000, CRC(af9e824e) SHA1(2b68813bf025a34b8958033108e4f8d39fd618cb) )
	ROM_LOAD( "mmp_u76.bin", 0x080000, 0x80000, CRC(c42af064) SHA1(f9d755e7cb52828d8594f7871932daf11443689f) )
	ROM_LOAD( "mmp_u77.bin", 0x100000, 0x80000, CRC(1d7396e1) SHA1(bde7e925051408dd2371b5da8235a6a4cae8cf6a) )
	ROM_LOAD( "mmp_u78.bin", 0x180000, 0x80000, CRC(5508d80b) SHA1(1b9a70a502b237fa11d1d55dce761e2def18873a) )
ROM_END

ROM_START( 3in1semi )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u52",  0x00001, 0x40000, CRC(b0e4a0f7) SHA1(e1f8b8ef020a85fcd7817814cf6c5d560e9e608d) )
	ROM_LOAD16_BYTE( "u74",  0x00000, 0x40000, CRC(266862c4) SHA1(2c5c513fee99bdb6e0ae3e0e644e516bdaddd629) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u35", 0x00000, 0x10000 , CRC(e40481da) SHA1(1c1fabcb67693235eaa6ff59ae12a35854b5564a) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
       shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(85deba7c) SHA1(44c6d9306b4f8e47182f4740a18971c49a8df8db) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u14", 0x00000, 0x40000, CRC(c83c11be) SHA1(c05d96d61e5b8245232c85cbbcb7cc1e4e066492) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u75", 0x000000, 0x80000, CRC(b66a0db6) SHA1(a4e604eb3c0a5b16b4b0bb99219045bf2146287c) )
	ROM_LOAD( "u76", 0x080000, 0x80000, CRC(5f4b48ea) SHA1(e9dd1100d55b021b060990988c1e5271ce1ae35b) )
	ROM_LOAD( "u77", 0x100000, 0x80000, CRC(d44211e3) SHA1(53af19dec03e76912632450414cdbcbb31cc094c) )
	ROM_LOAD( "u78", 0x180000, 0x80000, CRC(af596afc) SHA1(875d7a51ff5c741cae4483d8da33df9cae8de52a) )
ROM_END

ROM_START( cookbib2 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "cookbib2.02",  0x00001, 0x40000, CRC(b2909460) SHA1(2438638af870cfc105631d2b5e5a27a64ab5394d) )
	ROM_LOAD16_BYTE( "cookbib2.01",  0x00000, 0x40000, CRC(65aafde2) SHA1(01f9f261527c35182f0445d641d987aa86ad750f) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "cookbib2.07", 0x00000, 0x10000 , CRC(f59f1c9a) SHA1(2830261fd55249e015514fcb4cf8392e83b7fd0d) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped */

	ROM_REGION( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
       shared ram, the MCU puts it there */
	ROM_LOAD16_WORD_SWAP( "protdata.bin", 0x00000, 0x200 , CRC(ae6d8ed5) SHA1(410cdacb9b90ea345c0e4be85e60a138f45a51f1) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "cookbib2.06", 0x00000, 0x20000, CRC(5e6f76b8) SHA1(725800143dfeaa6093ed5fcc5b9f15678ae9e547) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "cookbib2.05", 0x000000, 0x80000, CRC(89fb38ce) SHA1(1b39dd9c2743916b8d8af590bd92fe4819c2454b) )
	ROM_LOAD( "cookbib2.04", 0x080000, 0x80000, CRC(f240111f) SHA1(b2c3b6e3d916fc68e1fd258b1279b6c39e1f0108) )
	ROM_LOAD( "cookbib2.03", 0x100000, 0x40000, CRC(e1604821) SHA1(bede6bdd8331128b9f2b229d718133470bf407c9) )
ROM_END

ROM_START( cookbib3 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u52.bin",  0x00001, 0x40000, CRC(65134893) SHA1(b1f26794d1a85893aedf55adb2195ad244f90132) )
	ROM_LOAD16_BYTE( "u74.bin",  0x00000, 0x40000, CRC(c4ab8435) SHA1(7f97d3deafb3eb5412a44308ef20d3317405e94c) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u35.bin", 0x0c000, 0x4000 ,  CRC(5dfd2a98) SHA1(193c0cd9272144c25cbc3660967424d34d0da185) ) /* bit strange but verified, not the first time SemiCom have done this, see bcstory.. */
	ROM_CONTINUE(0x8000,0x4000)
	ROM_CONTINUE(0x4000,0x4000)
	ROM_CONTINUE(0x0000,0x4000)

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
       shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(c819b9a8) SHA1(1d425e8c9940c0e691149e5406dd71808bd73832) )
	/* the 'empty' pattern continued after 0x200 but the game doesn't use it or attempt to decrypt it */

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u14.bin", 0x00000, 0x20000, CRC(e5bf9288) SHA1(12fb9542f9105fe1a21a74a08cda4d6372b984ee) )

	ROM_REGION( 0x180000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u75.bin", 0x000000, 0x80000, CRC(cbe4d9c8) SHA1(81b043bd2b45ab2a8c9df0ba599c6220ed0c9fbf) )
	ROM_LOAD( "u76.bin", 0x080000, 0x80000, CRC(1be17b57) SHA1(57b58cc094d6b47ed6136266f1d34b8bad3f421f) )
	ROM_LOAD( "u77.bin", 0x100000, 0x80000, CRC(7823600d) SHA1(90d431f324b71758c49f3a72ee07701ceb76403f) )
ROM_END

ROM_START( 4in1boot ) /* snow bros, tetris, hyperman 1, pacman 2 */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u52",  0x00001, 0x80000, CRC(71815878) SHA1(e3868f5687c1d8ec817671c50ade6c56ee83bfa1) )
	ROM_LOAD16_BYTE( "u74",  0x00000, 0x80000, CRC(e22d3fa2) SHA1(020ab92d8cbf37a9f8186a81934abb97088c16f9) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u35", 0x00000, 0x10000 , CRC(c894ac80) SHA1(ee896675b5205ab2dbd0cbb13db16aa145391d06) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u14", 0x00000, 0x40000, CRC(94b09b0e) SHA1(414de3e36eff85126038e8ff74145b35076e0a43) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u78", 0x000000, 0x200000, CRC(6c1fbc9c) SHA1(067f32cae89fd4d57b90be659d2d648e557c11df) )
ROM_END

ROM_START( snowbros3 )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "ur4",  0x00000, 0x20000, CRC(19c13ffd) SHA1(4f9db70354bd410b7bcafa96be4591de8dc33d90) )
	ROM_LOAD16_BYTE( "ur3",  0x00001, 0x20000, CRC(3f32fa15) SHA1(1402c173c1df142ff9dd7b859689c075813a50e5) )

	/* the sound is driven by a PIC? */
	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "sound.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "ua5",		0x000000, 0x80000, CRC(0604e385) SHA1(96acbc65a8db89a7be100f852dc07ba9a0313167) )	/* 16x16 tiles */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* 16x16 BG Tiles */
	ROM_LOAD( "un7",		0x000000, 0x200000, CRC(4a79da4c) SHA1(59207d116d39b9ee25c51affe520f5fdff34e536) )
	ROM_LOAD( "un8",		0x200000, 0x200000, CRC(7a4561a4) SHA1(1dd823369c09368d1f0ec8e1cb85d700f10ff448) )

	ROM_REGION( 0x100000, "oki", 0 )	/* OKIM6295 samples */
	ROM_LOAD( "us5",     0x00000, 0x20000, CRC(7c6368ef) SHA1(53393c570c605f7582b61c630980041e2ed32e2d) )
	ROM_CONTINUE(0x80000,0x60000)
ROM_END

/*

Information from Korean arcade gaming magazine

name : Final Tetris
author : Jeil computer system
year : 1993.08.24

*/

ROM_START( finalttr )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "10.7o",    0x00000, 0x20000, CRC(eecc83e5) SHA1(48088a2fae8852a73a325a9659c24b241515eac3) )
	ROM_LOAD16_BYTE( "9.5o",     0x00001, 0x20000, CRC(58d3640e) SHA1(361bc64174a6c7b15a13e0d1f048c7ea270182ca) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "12.5r",    0x00000, 0x10000, CRC(4bc21361) SHA1(dab9bea665c0f2fd7cee8ab7f3762e427911bcca) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped */

	ROM_REGION( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
       shared ram, the MCU puts it there */
	ROM_LOAD16_WORD_SWAP( "protdata.bin", 0x00000, 0x200 , CRC(d5bbb006) SHA1(2f9ce6c4f4f5a304a807134da9c85c68a7b49200) )
	/* after 0xc7 the data read seems meaningless garbage, it doesn't appear to
       stop at 0x102200, might be worth going back and checking if its simply random
       values due to ram not being cleared, or actual data */

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "11.7p",    0x00000, 0x20000, CRC(2e331022) SHA1(1e74c66d16eb9c8ae04acecbb4040dea037492cc) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "5.1d",     0x00000, 0x40000, CRC(64a450f3) SHA1(d0560f68fe1527fda7852269ec39237ace66ab32) )
	ROM_LOAD( "6.1f",     0x40000, 0x40000, CRC(7281a3cc) SHA1(3f2ed7893bd7c5ff25ecb6eabce78ab66fe532a7) )
	ROM_LOAD( "7.1g",     0x80000, 0x40000, CRC(ec80f442) SHA1(870e44d28490a324f74af554604b9daa8422b86f) )
	ROM_LOAD( "9.1h",     0xc0000, 0x40000, CRC(2ebd316d) SHA1(2f1249ebd2a0bb0cc15259f7187201576a365fa6) )
ROM_END

static READ16_HANDLER ( moremorp_0a_read )
{
	return 0x000a;
}

static DRIVER_INIT( moremorp )
{
//  UINT16 *PROTDATA = (UINT16*)memory_region(machine, "user1");
//  int i;

//  for (i = 0;i < 0x200/2;i++)
//      hyperpac_ram[0xf000/2 + i] = PROTDATA[i];

	/* explicit check in the code */
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x200001, 0, 0, moremorp_0a_read );
}


static DRIVER_INIT( cookbib2 )
{
//  UINT16 *HCROM = (UINT16*)memory_region(machine, "maincpu");
//  UINT16 *PROTDATA = (UINT16*)memory_region(machine, "user1");
//  int i;
//  hyperpac_ram[0xf000/2] = 0x46fc;
//  hyperpac_ram[0xf002/2] = 0x2700;

// verified on real hardware, need to move this to a file really

//  static UINT16 cookbib2_mcu68k[] =
//  {
//      // moved to protdata.bin
//  };




//for (i = 0;i < sizeof(cookbib2_mcu68k)/sizeof(cookbib2_mcu68k[0]);i++)
//      hyperpac_ram[0xf000/2 + i] = cookbib2_mcu68k[i];

//  for (i = 0;i < 0x200/2;i++)
//      hyperpac_ram[0xf000/2 + i] = PROTDATA[i];


	// trojan is actually buggy and gfx flicker like crazy
	// but we can pause the system after bootup with HALT line of 68k to get the table before
	// it goes nuts

	//  hyperpac_ram[0xf07a/2] = 0x4e73;
	//  hyperpac_ram[0xf000/2] = 0x4e73;

#if 0

	/* interrupt wait loop? */
	HCROM[0x014942/2] = 0x4eb9;
	HCROM[0x014944/2] = 0x0004;
	HCROM[0x014946/2] = 0x8000;
	HCROM[0x014948/2] = 0x4e71;

	/* interrupt wait loop? */
	HCROM[0x014968/2] = 0x4eb9;
	HCROM[0x01496a/2] = 0x0004;
	HCROM[0x01496c/2] = 0x8100;
	HCROM[0x01496e/2] = 0x4e71;

	/* interrupt wait loop? */
	HCROM[0x014560/2] = 0x4eb9;
	HCROM[0x014562/2] = 0x0004;
	HCROM[0x014564/2] = 0x8200;
	HCROM[0x014566/2] = 0x4e71;

	/* new code for interrupt wait */
	HCROM[0x048000/2] = 0x4a79;
	HCROM[0x048002/2] = 0x0010;
	HCROM[0x048004/2] = 0x2462;
	HCROM[0x048006/2] = 0x66f8;
	HCROM[0x048008/2] = 0x4eb9;
	HCROM[0x04800a/2] = 0x0004;
	HCROM[0x04800c/2] = 0x8300;
	HCROM[0x04800e/2] = 0x4e75;

	/* new code for interrupt wait */
	HCROM[0x048100/2] = 0x4a79;
	HCROM[0x048102/2] = 0x0010;
	HCROM[0x048104/2] = 0x2460;
	HCROM[0x048106/2] = 0x66f8;
	HCROM[0x048108/2] = 0x4eb9;
	HCROM[0x04810a/2] = 0x0004;
	HCROM[0x04810c/2] = 0x8300;
	HCROM[0x04810e/2] = 0x4e75;

	/* new code for interrupt wait */
	HCROM[0x048200/2] = 0x4a79;
	HCROM[0x048202/2] = 0x0010;
	HCROM[0x048204/2] = 0x2490;
	HCROM[0x048206/2] = 0x66f8;
	HCROM[0x048208/2] = 0x4eb9;
	HCROM[0x04820a/2] = 0x0004;
	HCROM[0x04820c/2] = 0x8300;
	HCROM[0x04820e/2] = 0x4e75;



	/* put registers on stack */
	HCROM[0x048300/2] = 0x48e7;
	HCROM[0x048302/2] = 0xfffe;

	/* wipe sprite ram (fill with 0x0002) */

	/* put the address we want to write TO in A2 */
	HCROM[0x048304/2] = 0x45f9;
	HCROM[0x048306/2] = 0x0070;
	HCROM[0x048308/2] = 0x0000;

	/* put the number of words we want to clear into D0 */
	HCROM[0x04830a/2] = 0x203c;
	HCROM[0x04830c/2] = 0x0000;
	HCROM[0x04830e/2] = 0x1000;

	/* write 0x0002 to A2 */
	HCROM[0x048310/2] = 0x34bc;
	HCROM[0x048312/2] = 0x0002;


	/* add 1 to write address a2 */
	HCROM[0x048314/2] = 0xd5fc;
	HCROM[0x048316/2] = 0x0000;
	HCROM[0x048318/2] = 0x0002;

	/* decrease counter d0 */
	HCROM[0x04831a/2] = 0x5380;

	/* compare d0 to 0 */
	HCROM[0x04831c/2] = 0x0c80;
	HCROM[0x04831e/2] = 0x0000;
	HCROM[0x048320/2] = 0x0000;

	/* if its not 0 then branch back */
	HCROM[0x048322/2] = 0x66ec;

	/* ram has been wiped */

	/* put the address we want to read protection data  in A2 */
	HCROM[0x048324/2] = 0x45f9;
	HCROM[0x048326/2] = 0x0010;
//  HCROM[0x048328/2] = 0xf000;
//  HCROM[0x048328/2] = 0xf000+0xb4;
	HCROM[0x048328/2] = 0xf000+0xb4+0xb4;

	/* put the address of spriteram  in A0 */
	HCROM[0x04832a/2] = 0x41f9;
	HCROM[0x04832c/2] = 0x0070;
	HCROM[0x04832e/2] = 0x0000;

	/* put the number of rows into D3 */
	HCROM[0x048330/2] = 0x263c;
	HCROM[0x048332/2] = 0x0000;
	HCROM[0x048334/2] = 0x0012;

	/* put the y co-ordinate of rows into D6 */
	HCROM[0x048336/2] = 0x2c3c;
	HCROM[0x048338/2] = 0x0000;
	HCROM[0x04833a/2] = 0x0014;

	/* put the number of bytes per row into D2 */
	HCROM[0x04833c/2] = 0x243c;
	HCROM[0x04833e/2] = 0x0000;
	HCROM[0x048340/2] = 0x000a;

	/* put the x co-ordinate of rows into D5 */
	HCROM[0x048342/2] = 0x2a3c;
	HCROM[0x048344/2] = 0x0000;
	HCROM[0x048346/2] = 0x0010;

	// move content of a2 to d4 (byte)
	HCROM[0x048348/2] = 0x1812;

	HCROM[0x04834a/2] = 0xe84c; // shift d4 right by 4

	HCROM[0x04834c/2] = 0x0244; // mask with 0x000f
	HCROM[0x04834e/2] = 0x000f; //

	/* jump to character draw to draw first bit */
	HCROM[0x048350/2] = 0x4eb9;
	HCROM[0x048352/2] = 0x0004;
	HCROM[0x048354/2] = 0x8600;

	// increase x-cord
	HCROM[0x048356/2] = 0x0645;
	HCROM[0x048358/2] = 0x000a;


	/* add 0x10 to draw address a0 */
	HCROM[0x04835a/2] = 0xd1fc;
	HCROM[0x04835c/2] = 0x0000;
	HCROM[0x04835e/2] = 0x0010;


	// move content of a2 to d4 (byte)
	HCROM[0x048360/2] = 0x1812;

	HCROM[0x048362/2] = 0x0244; // mask with 0x000f
	HCROM[0x048364/2] = 0x000f; //

	/* jump to character draw to draw second bit */
	HCROM[0x048366/2] = 0x4eb9;
	HCROM[0x048368/2] = 0x0004;
	HCROM[0x04836a/2] = 0x8600;

	// increase x-cord
	HCROM[0x04836c/2] = 0x0645;
	HCROM[0x04836e/2] = 0x000c;

	/* add 0x10 to draw address a0 */
	HCROM[0x048370/2] = 0xd1fc;
	HCROM[0x048372/2] = 0x0000;
	HCROM[0x048374/2] = 0x0010;

// newcode
	/* add 1 to read address a2 */
	HCROM[0x048376/2] = 0xd5fc;
	HCROM[0x048378/2] = 0x0000;
	HCROM[0x04837a/2] = 0x0001;

	/* decrease counter d2 (row count)*/
	HCROM[0x04837c/2] = 0x5382;

	/* compare d2 to 0 */
	HCROM[0x04837e/2] = 0x0c82;
	HCROM[0x048380/2] = 0x0000;
	HCROM[0x048382/2] = 0x0000;

	/* if its not 0 then branch back */
	HCROM[0x048384/2] = 0x66c2;

	// increase y-cord d6
	HCROM[0x048386/2] = 0x0646;
	HCROM[0x048388/2] = 0x000c;

	/* decrease counter d3 */
	HCROM[0x04838a/2] = 0x5383;

	/* compare d3 to 0 */
	HCROM[0x04838c/2] = 0x0c83;
	HCROM[0x04838e/2] = 0x0000;
	HCROM[0x048390/2] = 0x0000;

	/* if its not 0 then branch back */
	HCROM[0x048392/2] = 0x66a8;

	/* get back registers from stack*/
	HCROM[0x048394/2] = 0x4cdf;
	HCROM[0x048396/2] = 0x7fff;

	/* rts */
	HCROM[0x048398/2] = 0x4e75;

	/* Draw a character! */
	/* D6 = y-coordinate
       D5 = x-coordinate
       D4 = value to draw

       A0 = spriteram base */

	// 0002 0002 0002 0010 00xx 00yy 00nn 000n

	// 357c 0020 000c
	// 337c = a1
	// move.w #$20, (#$c, A2)

	HCROM[0x048600/2] = 0x317c;
	HCROM[0x048602/2] = 0x0010;
	HCROM[0x048604/2] = 0x0006;

	HCROM[0x048606/2] = 0x3145;
	HCROM[0x048608/2] = 0x0008;

	HCROM[0x04860a/2] = 0x3146;
	HCROM[0x04860c/2] = 0x000a;

/* get true value */

	/* put lookuptable address in  A3 */
	HCROM[0x04860e/2] = 0x47f9;
	HCROM[0x048610/2] = 0x0004;
	HCROM[0x048612/2] = 0x8800;

	HCROM[0x048614/2] = 0x3004; // d4 -> d0
	HCROM[0x048616/2] = 0xe348;

	HCROM[0x048618/2] = 0x3173;
	HCROM[0x04861a/2] = 0x0000;
	HCROM[0x04861c/2] = 0x000c;

/* not value */

	HCROM[0x04861e/2] = 0x317c;
	HCROM[0x048620/2] = 0x0000;
	HCROM[0x048622/2] = 0x000e;

	/* rts */
	HCROM[0x048624/2] = 0x4e75;


	/* table used for lookup by the draw routine to get real tile numbers */

	HCROM[0x048800/2] = 0x0010;
	HCROM[0x048802/2] = 0x0011;
	HCROM[0x048804/2] = 0x0012;
	HCROM[0x048806/2] = 0x0013;
	HCROM[0x048808/2] = 0x0014;
	HCROM[0x04880a/2] = 0x0015;
	HCROM[0x04880c/2] = 0x0016;
	HCROM[0x04880e/2] = 0x0017;
	HCROM[0x048810/2] = 0x0018;
	HCROM[0x048812/2] = 0x0019;
	HCROM[0x048814/2] = 0x0021;
	HCROM[0x048816/2] = 0x0022;
	HCROM[0x048818/2] = 0x0023;
	HCROM[0x04881a/2] = 0x0024;
	HCROM[0x04881c/2] = 0x0025;
	HCROM[0x04881e/2] = 0x0026;



/*
10 0
11 1
12 2
13 3
14 4
15 5
16 6
17 7
18 8
19 9
21 a
22 b
23 c
24 d
25 e
26 f
*/





	{
		FILE *fp;

		fp=fopen("cookie", "w+b");
		if (fp)
		{
			fwrite(HCROM, 0x80000, 1, fp);
			fclose(fp);
		}
	}
#endif
}

// protection simulation no longer used
#if 0
static DRIVER_INIT( hyperpac )
{
	/* simulate RAM initialization done by the protection MCU */
	/* not verified on real hardware */
	hyperpac_ram[0xe000/2] = 0x4ef9;
	hyperpac_ram[0xe002/2] = 0x0000;
	hyperpac_ram[0xe004/2] = 0x062c;

	hyperpac_ram[0xe080/2] = 0xfedc;
	hyperpac_ram[0xe082/2] = 0xba98;
	hyperpac_ram[0xe084/2] = 0x7654;
	hyperpac_ram[0xe086/2] = 0x3210;
}
#endif

static READ16_HANDLER ( _4in1_02_read )
{
	return 0x0202;
}

static DRIVER_INIT(4in1boot)
{
	UINT8 *buffer;
	UINT8 *src = memory_region(machine, "maincpu");
	int len = memory_region_length(machine, "maincpu");

	/* strange order */
	buffer = auto_alloc_array(machine, UINT8, len);
	{
		int i;
		for (i = 0;i < len; i++)
			if (i&1) buffer[i] = BITSWAP8(src[i],6,7,5,4,3,2,1,0);
			else buffer[i] = src[i];

		memcpy(src,buffer,len);
		auto_free(machine, buffer);
	}

	src = memory_region(machine, "soundcpu");
	len = memory_region_length(machine, "soundcpu");

	/* strange order */
	buffer = auto_alloc_array(machine, UINT8, len);
	{
		int i;
		for (i = 0;i < len; i++)
			buffer[i] = src[i^0x4000];
		memcpy(src,buffer,len);
		auto_free(machine, buffer);
	}
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x200001, 0, 0, _4in1_02_read );
}

static DRIVER_INIT(snowbro3)
{
	UINT8 *buffer;
	UINT8 *src = memory_region(machine, "maincpu");
	int len = memory_region_length(machine, "maincpu");

	/* strange order */
	buffer = auto_alloc_array(machine, UINT8, len);
	{
		int i;
		for (i = 0;i < len; i++)
			buffer[i] = src[BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,3,4,1,2,0)];
		memcpy(src,buffer,len);
		auto_free(machine, buffer);
	}
}

static READ16_HANDLER( _3in1_read )
{
	return 0x0a0a;
}

static DRIVER_INIT( 3in1semi )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x200001, 0, 0, _3in1_read );
}

static READ16_HANDLER( cookbib3_read )
{
	return 0x2a2a;
}

static DRIVER_INIT( cookbib3 )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x200001, 0, 0, cookbib3_read );
}

static DRIVER_INIT( pzlbreak )
{
	pandora_set_bg_pen(machine->device("pandora"), 0xc0);
}

GAME( 1990, snowbros,  0,        snowbros, snowbros, 0, ROT0, "Toaplan", "Snow Bros. - Nick & Tom (set 1)", 0 )
GAME( 1990, snowbrosa, snowbros, snowbros, snowbros, 0, ROT0, "Toaplan", "Snow Bros. - Nick & Tom (set 2)", 0 )
GAME( 1990, snowbrosb, snowbros, snowbros, snowbros, 0, ROT0, "Toaplan", "Snow Bros. - Nick & Tom (set 3)", 0 )
GAME( 1990, snowbrosc, snowbros, snowbros, snowbros, 0, ROT0, "Toaplan", "Snow Bros. - Nick & Tom (set 4)", 0 )
GAME( 1990, snowbrosj, snowbros, snowbros, snowbroj, 0, ROT0, "Toaplan", "Snow Bros. - Nick & Tom (Japan)", 0 )
GAME( 1990, snowbrosd, snowbros, snowbros, snowbroj, 0, ROT0, "Toaplan (Dooyong license)", "Snow Bros. - Nick & Tom (Dooyong license)", 0 )
GAME( 1990, wintbob,   snowbros, wintbob,  snowbros, 0, ROT0, "bootleg (Sakowa Project Korea)", "The Winter Bobble (bootleg of Snow Bros.)", 0 )

// none of the games below are on genuine SnowBros hardware, but they clone the functionality of it.
GAME( 1993, finalttr, 0,        finalttr, finalttr, 0,        ROT0, "Jeil Computer System", "Final Tetris", 0 )
GAME( 1995, honeydol, 0,        honeydol, honeydol, 0, ROT0, "Barko Corp.", "Honey Dolls", 0 ) // based on snowbros code..
GAME( 1995, twinadv,  0,        twinadv,  twinadv,  0, ROT0, "Barko Corp.", "Twin Adventure (World)", 0 )
GAME( 1995, twinadvk, twinadv,  twinadv,  twinadv,  0, ROT0, "Barko Corp.", "Twin Adventure (Korea)", 0 )
GAME( 1995, hyperpac, 0,        semicom_mcu,  hyperpac, 0, ROT0, "SemiCom", "Hyper Pacman", 0 )
GAME( 1995, hyperpacb,hyperpac, semicom,  hyperpac, 0,        ROT0, "bootleg", "Hyper Pacman (bootleg)", 0 )
GAME( 1996, cookbib2, 0,        semiprot, cookbib2, cookbib2, ROT0, "SemiCom", "Cookie & Bibi 2", 0 )
GAME( 1996, toppyrap, 0,        semiprot, toppyrap, 0,        ROT0, "SemiCom", "Toppy & Rappy", 0 )
GAME( 1997, cookbib3, 0,        semiprot, cookbib3, cookbib3, ROT0, "SemiCom", "Cookie & Bibi 3", 0 )
GAME( 1997, 3in1semi, 0,        semiprot, moremore, 3in1semi, ROT0, "SemiCom", "XESS - The New Revolution (SemiCom 3-in-1)", 0 )
GAME( 1997, twinkle,  0,        semiprot, moremore, 0,        ROT0, "SemiCom", "Twinkle", 0 )
GAME( 1997, pzlbreak, 0,        semiprot, pzlbreak, pzlbreak, ROT0, "SemiCom", "Puzzle Break", 0 )
GAME( 1999, moremore, 0,        semiprot, moremore, moremorp, ROT0, "SemiCom / Exit", "More More", 0 )
GAME( 1999, moremorp, 0,        semiprot, moremore, moremorp, ROT0, "SemiCom / Exit", "More More Plus", 0 )
GAME( 2002, 4in1boot, 0,        _4in1,    4in1boot, 4in1boot, ROT0, "K1 Soft", "Puzzle King (includes bootleg of Snow Bros.)" , 0)
GAME( 2002, snowbros3,snowbros, snowbro3, snowbroj, snowbro3, ROT0, "Syrmex", "Snow Brothers 3 - Magical Adventure", GAME_IMPERFECT_SOUND ) // its basically snowbros code?...
