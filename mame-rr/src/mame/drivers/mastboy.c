/*
    Master Boy - Gaelco (c)1991

    MAME Driver by David Haywood

    Special Thanks to Charles MacDonald
                  and ClawGrip

    Notes:

    Why does RAM-M fail on first boot, Charles mentioned it can (randomly?) fail on real HW too, is it buggy code?
    Are the correct/incorrect samples when you answer a question meant to loop as they do?
    Video timing should be hooked up with MAME's new video timing system

    Are the glitches on the startup screen in the Italian version caused by the original
    having a different internal program, or is it just a (bad) hack?

    This Can be converted to tilemaps very easily, but probably not worth it

    Backup RAM enable / disable needs figuring out properly
*/

/*
    -----------------------------
     Hardware Notes (from f205v)
    -----------------------------

    Master Boy

    Produttore  Gaelco
    N.revisione rev A - Italian
    CPU
    1x HD647180X0CP6-1M1R (main)(on a small piggyback)
    1x SAA1099P (sound)
    1x OKI5205 (sound)
    1x crystal resonator POE400B (close to sound)
    1x oscillator 24.000000MHz (close to main)
    ROMs
    1x M27C2001 (1)
    4x AM27C010 (2,5,6,7)
    2x TMS27C512 (3,4)

*/

/*
    ------------------------------------------------
     Technical Information (from Charles MacDonald)
    ------------------------------------------------

     ROM information

     'hd647180.bin' is a dump of the internal 16K ROM from the Master Boy CPU
     module, stamped with:

     "MASTER-BOY"
     "REV- A"

     .. on the back. This is the Spanish version of the game.

     Technical stuff

     The MMU is hardly used, but the game does ensure the MMU registers
     are set up when an NMI occurs, so that needs to be emulated. Otherwise
     it jumps to address zero to force a reset.

     CPU memory map:

     0000-3FFF : HD647180 internal ROM
     4000-7FFF : Last 16K of 'S.MASTER 3*' ROM
     8000-8FFF : Name table RAM (4K)
     9000-9FFF : Work RAM (4K)
     A000-AFFF : Color RAM
     B000-B7FF : EEPROM (28C16, 2Kx8)
     B800-BBFF : I/O area
     BC00-BFFF : Unused; MCU maps it's internal 512-byte RAM here.
     C000-FFFF : Banked memory area

     I/O area

     The 2K area is split into a 64 byte section repeatedly mirrored throughout
     the 2K range, and the 64 byte section is split into 8 byte sections mirrored
     throughout the 64 bytes:

     B800-B807 : Input port #1 (r/o)
     B808-B80F : Input port #2 (r/o)
     B810-B817 : DIP switch #1 (r/o)
     B818-B81F : DIP switch #2 (r/o)
     B820-B827 : Bank control latch: (w/o)
                 D7 : 1= Access ROMs    | 0= Access VRAM/VROM
                 D6 : ROM select bit 2  | ?
                 D5 : ROM select bit 1  | ?
                 D4 : ROM select bit 0  | ?
                 D3 : Common ROM A17    | ?
                 D2 : Common ROM A16    | ?
                 D1 : Common ROM A15    | VRAM/VROM A15
                 D0 : Common ROM A14    | VRAM/VROM A14

                 ROM select bits:

                 0= "S.MASTER 1" (27C020)
                 1= "S.MASTER 2" (27C010)
                 2= "S.MASTER 5" (27C010)
                 3= "S.MASTER 6" (27C010)
                 4= "S.MASTER 7" (27C010)
                 5= "S.MASTER 8" (27C010)
                 6= Empty socket
                 7= Empty socket

                 The common ROM banking bits address a 256K space; if a 128K ROM
                 is selected then it appears to be mirrored twice.

                 The game may scan the empty socket area when it checks
                 the question data in the ROMs during startup. Not sure
                 what value should be returned, most likely the last byte
                 that was left on the data bus is still present.

     B828-B82F : SAA1099 PSG
     B830-B837 : ADPCM data latch (w/o)
     B838-B83F : Output latches (w/o, only bit 0 of each address is latched)
          B838 : IC30 pin 10 (write to this to acknowledge /INT0 interrupt)
          B839 : MSM5205 S1 (sample rate select, bit 0)
          B83A : MSM5205 S2 (sample rate select, bit 1)
          B83B : MSM5205 RESET
          B83C : IC46 p11 (write to this to enable EEPROM)
          B83D : ?
          B83E : ?
          B83F : ?

     EEPROM

     The EEPROM has a really weird write-protect, I think any access to it
     disables it so that you have to continuously write to the output
     port latch listed above for every read/write. The circuit for this
     is a total mess.

     Timing

     - HD647180 runs at 12.000 / 2 MHz
     - SAA1099 PSG runs at 6.000 MHz
     - Display pixel clock is 6.000 MHz
     - Oki MSM5205 has a 384 KHz oscillator, this is the standard setup
       so S1,S2 select the same frequencies listed in the datasheet.

     Interrupts

     /INT0 from the scanline counter, probably once per frame but it could
     be ever couple of lines. Needs to be acknowledged by writing to the
     output port latch listed above.

     /NMI from the MSM5205 on every other sample, see below:

     Sound hardware

     SAA1099 PSG has two addresses at $B828,$B829.

     Oki MSM5205 that is driven by the CPU:

     Write to $B830 to load 8-bit latch with two ADPCM samples.
     When MSM5205 RESET is inactive, it automatically generates VCKs at the
     rate specified by S1, S2. On the first VCK a nibble of the ADPCM
     latch is output to the MSM5205. On the second VCK the next nibble
     is output and an NMI is triggered. The NMI handler should load the next
     byte of ADPCM data into $B830.

     Video hardware

     No sprites, one tiled 32x32 background. Tiles use packed nibbles to
     provide 4 bits of data, a palette provides the other 4 to index 256
     color RAM entries that are 12-bits of RGB each (more info follows)
     Tiles come from video RAM (64K) or video ROM (64K) mapped to a 128K space:

     00000-07FFF : 32K RAM (IC90)
     08000-0FFFF : 32K RAM (IC89)
     10000-1FFFF : 64K ROM ("S.MASTER 4")

     The CPU can access this region in 16K banks to read the ROM or read/write
     the RAM.

     The name table RAM is a 32x32 matrix of 4-byte entries. I don't have the
     exact layout but there are at least 12 bits of tile number, 4 bits of
     palette select, and possibly 2 bits for tile flipping.

     The name table RAM mapping to the screen is a little odd. Rows are stored
     in reverse order. (31 to 0 rather than 0 to 31). Of each row, it looks
     like the first 64 bytes define the right half of the row, the next 64
     bytes define the left half.

     Video timing

     Horizontal timing

      84 pixels black right border
     256 pixels active display
      12 pixels black left border
      32 pixels horizontal sync pulse width
     384 pixels total

     Vertical timing

     224 scanlines active display
      58 scanlines vblank + vsync (don't have the exact numbers for top/bottom border)
     282 scanlines total

     Pixel clock is 6.000 MHz, so:

     6.000 / 384 pixels per line / 282 lines per frame = 55.408 Hz refresh rate

     Palette

     There is 1K of color RAM, arranged as 256 words of 16-bit data with
     only 12 bits used:

     (MSB)
     lCD7 - Green bit 3
     lCD6 - Green bit 2
     lCD5 - Green bit 1
     lCD4 - Green bit 0
     lCD3 - Red bit 3
     lCD2 - Red bit 2
     lCD1 - Red bit 1
     lCD0 - Red bit 0
     (LSB)
      CD7 - Blue bit 3
      CD6 - Blue bit 2
      CD5 - Blue bit 1
      CD4 - Blue bit 0
      CD3 - Not used
      CD2 - Not used
      CD1 - Not used
      CD0 - Not used

     So white is $FFF0, etc. The lowest nibble is not connected to anything
     and has no purpose.

     Color DAC resistor weighting is:

     Color bit 0 is 2K ohms
     Color bit 1 is 1K ohms
     Color bit 2 is 500 ohms
     Color bit 3 is 250 ohms

     About the dump

     Bytes that were 0x7F or 0x5C couldn't be dumped, and read as 0x23.
     The ROM has been patched with the correct data, this is just for reference
     in case something seems wrong. The address checking and patching was
     automatic so there aren't any human errors:

     Potential bad addresses that are either 0x7F or 0x5C:

        0x0300,
        0x0534,
        0x05B1,
        0x05B5,
        0x0E4E,
        0x1090,
        0x10E0,
        0x110F,
        0x11B5,
        0x1259,
        0x1271,
        0x1278,
        0x14AC,
        0x14CC,
        0x14DC,
        0x15D9,
        0x178D,
        0x17AA,
        0x17B4,
        0x186C,
        0x1884,
        0x1A0A,
        0x1A66,
        0x1ABB,
        0x1AE9,
        0x1CAD,
        0x1CB9,
        0x1CC1,
        0x1CCF,
        0x1CD5,
        0x1CE3,
        0x1CF3,
        0x1D85,
        0x1DC7,
        0x1E00,
        0x1E26,
        0x1E2A,
        0x1E5E,
        0x1E6F,
        0x1E89,
        0x1EA2,
        0x1EC9,
        0x1F14,
        0x1F23,
        0x1F33,
        0x1F4D,
        0x1F61,
        0x1FE3,
        0x1FEA,
        0x1FFD,
        0x200D,
        0x2046,
        0x2054,
        0x2069,
        0x206F,
        0x207D,
        0x208A,
        0x20A2,
        0x20BA,
        0x2149,
        0x215D,
        0x2179,
        0x2189,
        0x21A7,
        0x21D7,
        0x21F2,
        0x2211,
        0x23E6,
        0x23EA,
        0x23F4,
        0x2442,
        0x2558,
        0x2559,
        0x2569,
        0x2586,
        0x258C,
        0x2592,
        0x2612,
        0x2613,
        0x26BF,
        0x26FF,
        0x2706,
        0x27A7,
        0x27AB,
        0x27BE,
        0x2867,
        0x2896,
        0x2977,
        0x29E2,
        0x2A03,
        0x2A79,
        0x2AA3,
        0x2ACA,
        0x2AE9,
        0x2C1A,
        0x2C30,
        0x2C41,
        0x2C4A,
        0x2C8D,
        0x2CAC,
        0x2D3A,
        0x2D88,
        0x2DAC,
        0x2E09,
        0x2EBD,
        0x2EF2,
        0x2F26,
        0x2F8B,
        0x2FA6,
        0x2FB1,
        0x2FC9,
        0x2FDA,
        0x2FE6,
        0x30FE,
        0x3101,
        0x31A3,
        0x31C0,
        0x31C7,
        0x3213,
        0x323D,
        0x3247,
        0x3271,
        0x32B3,
        0x332D,
        0x333A,
        0x334F,
        0x33C7,
        0x33F1,
        0x3430,
        0x345D,
        0x3467,
        0x3478,
        0x351D,
        0x353F,
        0x3540,
        0x36CC,
        0x36D1,
        0x37C5,
        0x37C9,
        0x3806,
        0x3944,
        0x394C,
        0x39CD,
        0x3A39,
        0x3A99,
        0x3A9F,
        0x3AC8,
        0x3B05,
        0x3B08,
        0x3B15,
        0x3B4A,
        0x3B62,
        0x3B8F,
        0x3C5F,
        0x3D36,
        0x3D8B,
        0x3E62

     Addresses from the above which were determined to be 0x5C:

        0x05B1, // 0x5C, newline
        0x0E4E, // 0x5C, newline
        0x1259, // 0x5C, newline
        0x1271, // 0x5C, newline
        0x1278, // 0x5C, newline
        0x1D85, // 0x5C, newline
        0x2189, // 0x5C, newline
        0x21D7, // 0x5C, newline
        0x23EA, // 0x5C, newline
        0x2442, // 0x5C, newline
        0x2559, // 0x5C, newline
        0x2586, // 0x5C, newline
        0x258C, // 0x5C, newline
        0x2592, // 0x5C, newline
        0x2613, // 0x5C, newline
        0x3101, // 0x5C, newline
        0x351D, // 0x5C, newline
        0x3540, // 0x5C, newline
        0x394C, // 0x5C, newline
        0x3D8B, // 0x5C, newline



*/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "sound/saa1099.h"
#include "sound/msm5205.h"

/* RAM areas */
static UINT8* mastboy_tileram;
static UINT8* mastboy_vram;
static UINT8* mastboy_colram;
static UINT8* mastboy_workram;

/* Bank Control */
static UINT8 mastboy_bank;

/* General */
static int mastboy_irq0_ack;
static int mastboy_backupram_enabled;

/* MSM5205 */
static int mastboy_m5205_next;
static int mastboy_m5205_part;
static int mastboy_m5205_sambit0, mastboy_m5205_sambit1;

/* VIDEO EMULATION */

static VIDEO_START(mastboy)
{
	gfx_element_set_source(machine->gfx[0], mastboy_vram);
}

static VIDEO_UPDATE(mastboy)
{
	int y,x,i;
	int count = 0x000;

	for (i=0;i<0x200;i+=2)
	{
		int coldat = mastboy_colram[i+1] |  (mastboy_colram[i+0]<<8);

		palette_set_color_rgb(screen->machine,i/2,pal4bit(coldat>>8),pal4bit(coldat>>12),pal4bit(coldat>>4));
	}

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			/* bytes 0 and 3 seem to be unused for rendering , they appear to contain data the game uses internally */
			int tileno = (mastboy_tileram[count+1]|(mastboy_tileram[count+2]<<8))&0xfff;
			int attr = (mastboy_tileram[count+2]&0xf0)>>4;
			gfx_element *gfx;

			if (tileno&0x800)
			{
				gfx = screen->machine->gfx[1];
				tileno &=0x7ff;
			}
			else
			{
				gfx = screen->machine->gfx[0];
			}


			drawgfx_opaque(bitmap,cliprect,gfx,tileno,attr,0,0,x*8,y*8);

			count+=4;

		}

	}

	return 0;
}


/* Access to Banked RAM */

static READ8_HANDLER(banked_ram_r)
{
	if ((mastboy_bank&0x80) == 0x00)
	{
		int bank;
		bank = mastboy_bank & 0x07;
		//if (mastboy_bank&0xf8) printf("invalid bank bits in vram/vrom read\n");

		if (bank>0x3) // ROM access
		{
			UINT8 *src    = memory_region( space->machine, "gfx1" );
			bank &=0x3;
			return src[offset+(bank*0x4000)];
		}
		else
		{
			bank &=0x3;
			/* we have to invert the data for the GFX decode */
			return mastboy_vram[offset+(bank*0x4000)]^0xff;
		}
	}
	else
	{
		UINT8 *src;
		int bank;
		bank = mastboy_bank & 0x7f;
		src = memory_region       ( space->machine, "user1" ) + bank * 0x4000;
		return src[offset];
	}
}

static WRITE8_HANDLER( banked_ram_w )
{
	if ((mastboy_bank&0x80) == 0x00)
	{
		int bank;
		bank = mastboy_bank & 0x07;
		//if (data&0xf8) printf("invalid bank bits in vram/vrom write\n");

		if (bank>0x3) // ROM access
		{
			logerror("Attempting to WRITE to VROM\n");
		}
		else
		{
			/* write to the RAM based tile data */
			int offs;
			bank &=0x3;

			offs = offset+(bank*0x4000);

			/* we have to invert the data for the GFX decode */
			mastboy_vram[offs] = data^0xff;

			/* Decode the new tile */
			gfx_element_mark_dirty(space->machine->gfx[0], offs/32);
		}
	}
	else
	{
		logerror("attempt to write %02x to banked area with BANKED ROM selected\n",data);
	}
}

static WRITE8_HANDLER( mastboy_bank_w )
{
	// controls access to banked ram / rom
	mastboy_bank = data;
}

/* Backup RAM access */

static READ8_HANDLER( mastboy_backupram_r )
{
	return space->machine->generic.nvram.u8[offset];
}

static WRITE8_HANDLER( mastboy_backupram_w )
{
//  if (mastboy_backupram_enabled)
//  {
		space->machine->generic.nvram.u8[offset] = data;
//  }
//  else
//  {
//      logerror("Write to BackupRAM when disabled! %04x, %02x\n", offset,data);
//  }
}

static WRITE8_HANDLER( backupram_enable_w )
{
	/* This is some kind of enable / disable control for backup ram (see Charles's notes) but I'm not
       sure how it works in practice, if we use it then it writes a lot of data with it disabled */
	mastboy_backupram_enabled = data&1;
}

/* MSM5205 Related */

static WRITE8_HANDLER( msm5205_mastboy_m5205_sambit0_w )
{
	running_device *adpcm = space->machine->device("msm");

	mastboy_m5205_sambit0 = data & 1;
	msm5205_playmode_w(adpcm,  (1 << 2) | (mastboy_m5205_sambit1 << 1) | (mastboy_m5205_sambit0) );

	logerror("msm5205 samplerate bit 0, set to %02x\n",data);
}

static WRITE8_HANDLER( msm5205_mastboy_m5205_sambit1_w )
{
	running_device *adpcm = space->machine->device("msm");

	mastboy_m5205_sambit1 = data & 1;

	msm5205_playmode_w(adpcm,  (1 << 2) | (mastboy_m5205_sambit1 << 1) | (mastboy_m5205_sambit0) );

	logerror("msm5205 samplerate bit 0, set to %02x\n",data);
}

static WRITE8_DEVICE_HANDLER( mastboy_msm5205_reset_w )
{
	mastboy_m5205_part = 0;
	msm5205_reset_w(device,data&1);
}

static WRITE8_HANDLER( mastboy_msm5205_data_w )
{
	mastboy_m5205_next = data;
}

static void mastboy_adpcm_int(running_device *device)
{
	msm5205_data_w(device, mastboy_m5205_next);
	mastboy_m5205_next >>= 4;

	mastboy_m5205_part ^= 1;
	if(!mastboy_m5205_part)
		cputag_set_input_line(device->machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE);
}


static const msm5205_interface msm5205_config =
{
	mastboy_adpcm_int,	/* interrupt function */
	MSM5205_SEX_4B		/* 4KHz 4-bit */
};

/* Interrupt Handling */

static WRITE8_HANDLER( mastboy_irq0_ack_w )
{
	mastboy_irq0_ack = data;
	if ((data & 1) == 1)
		cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
}

static INTERRUPT_GEN( mastboy_interrupt )
{
	if ((mastboy_irq0_ack & 1) == 1)
	{
		cpu_set_input_line(device, 0, ASSERT_LINE);
	}
}

/* Memory Maps */

static ADDRESS_MAP_START( mastboy_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM // Internal ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROM // External ROM

	AM_RANGE(0x8000, 0x8fff) AM_RAM AM_BASE(&mastboy_workram)// work ram
	AM_RANGE(0x9000, 0x9fff) AM_RAM AM_BASE(&mastboy_tileram)// tilemap ram
	AM_RANGE(0xa000, 0xa1ff) AM_RAM AM_BASE(&mastboy_colram) AM_MIRROR(0x0e00)  // colour ram

	AM_RANGE(0xc000, 0xffff) AM_READWRITE(banked_ram_r,banked_ram_w) // mastboy bank area read / write

	AM_RANGE(0xff000, 0xff7ff) AM_READWRITE(mastboy_backupram_r,mastboy_backupram_w) AM_BASE_SIZE_GENERIC(nvram)

	AM_RANGE(0xff800, 0xff807) AM_READ_PORT("P1")
	AM_RANGE(0xff808, 0xff80f) AM_READ_PORT("P2")
	AM_RANGE(0xff810, 0xff817) AM_READ_PORT("DSW1")
	AM_RANGE(0xff818, 0xff81f) AM_READ_PORT("DSW2")

	AM_RANGE(0xff820, 0xff827) AM_WRITE(mastboy_bank_w)
	AM_RANGE(0xff828, 0xff828) AM_DEVWRITE("saa", saa1099_data_w)
	AM_RANGE(0xff829, 0xff829) AM_DEVWRITE("saa", saa1099_control_w)
	AM_RANGE(0xff830, 0xff830) AM_WRITE(mastboy_msm5205_data_w)
	AM_RANGE(0xff838, 0xff838) AM_WRITE(mastboy_irq0_ack_w)
	AM_RANGE(0xff839, 0xff839) AM_WRITE(msm5205_mastboy_m5205_sambit0_w)
	AM_RANGE(0xff83a, 0xff83a) AM_WRITE(msm5205_mastboy_m5205_sambit1_w)
	AM_RANGE(0xff83b, 0xff83b) AM_DEVWRITE("msm", mastboy_msm5205_reset_w)
	AM_RANGE(0xff83c, 0xff83c) AM_WRITE(backupram_enable_w)

	AM_RANGE(0xffc00, 0xfffff) AM_RAM // Internal RAM
ADDRESS_MAP_END

/* Ports */

static READ8_HANDLER( mastboy_port_38_read )
{
	return 0x00;
}

static READ8_HANDLER( mastboy_nmi_read )
{
	// this is read in the NMI, it's related to the Z180 MMU I think, must return right value or game jumps to 0000
	return 0x00;
}

static ADDRESS_MAP_START( mastboy_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x38, 0x38) AM_READ(mastboy_port_38_read)
	AM_RANGE(0x39, 0x39) AM_READ(mastboy_nmi_read)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( mastboy )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x1e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Game Mode" )
	PORT_DIPSETTING(    0x01, "1" ) /* 1: Counts only the right or wrong answer from the player who answered first. */
	PORT_DIPSETTING(    0x00, "2" ) /* 2: Waits until both players have answered and then counts the right or wrong answers. */

	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) // Demo Sounds
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )

	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x0C, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_DIPNAME( 0x10, 0x10, "Erase Records" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )

	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_8C ) )

	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ) )
INPUT_PORTS_END

/* GFX Decodes */

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 24,28,16,20,8,12,0,4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};


static const gfx_layout tiles8x8_layout_2 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0,4,8,12,16,20,24,28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};


static GFXDECODE_START( mastboy )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2",  0, tiles8x8_layout_2, 0, 16 )
GFXDECODE_END

/* Machine Functions / Driver */

static MACHINE_RESET( mastboy )
{
	/* clear some ram */
	memset( mastboy_workram,   0x00, 0x01000);
	memset( mastboy_tileram,   0x00, 0x01000);
	memset( mastboy_colram,    0x00, 0x00200);
	memset( mastboy_vram, 0x00, 0x10000);

	mastboy_m5205_part = 0;
	msm5205_reset_w(machine->device("msm"),1);
	mastboy_irq0_ack = 0;
}



static MACHINE_DRIVER_START( mastboy )
	MDRV_CPU_ADD("maincpu", Z180, 12000000/2)	/* HD647180X0CP6-1M1R */
	MDRV_CPU_PROGRAM_MAP(mastboy_map)
	MDRV_CPU_IO_MAP(mastboy_io_map)
	MDRV_CPU_VBLANK_INT("screen", mastboy_interrupt)

	MDRV_NVRAM_HANDLER(generic_1fill)

	MDRV_MACHINE_RESET( mastboy )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(6000000.0f / 384.0f / 282.0f)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)

	MDRV_GFXDECODE(mastboy)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(mastboy)
	MDRV_VIDEO_UPDATE(mastboy)

	// sound hardware
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("saa", SAA1099, 6000000 )
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

/* Romsets */

ROM_START( mastboy )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hd647180.bin", 0x00000, 0x4000, CRC(75716dd1) SHA1(9b14b9b889b29b6022a3815de95487fb6a720d7a) ) // game code is internal to the CPU!
	ROM_LOAD( "03.bin",       0x04000, 0x4000, CRC(5020a37f) SHA1(8bc75623232f3ab457b47d5af6cd1c3fb24c0d0e) ) // sound data? (+ 1 piece of) 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(             0x04000, 0x4000 )
	ROM_CONTINUE(             0x04000, 0x4000 )
	ROM_CONTINUE(             0x04000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "gfx1", ROMREGION_ERASE00 ) /* RAM accessed by the video chip */
	/* 0x00000 - 0x0ffff = banked ram */

	ROM_REGION( 0x10000, "gfx2", ROMREGION_INVERT ) /* ROM accessed by the video chip */
	ROM_LOAD( "04.bin", 0x00000, 0x10000, CRC(565932f4) SHA1(4b184aa445b5671072031ad4a2ccb13868d6d3a4) )

	ROM_REGION( 0x200000, "user1", 0 ) /* banked data - 8 banks, 6 'question' slots */
	ROM_LOAD( "01.bin", 0x000000,   0x040000, CRC(36755831) SHA1(706fba5fc765502774643bfef8a3c9d2c01eb01b) ) // 99% gfx
	ROM_LOAD( "02.bin", 0x040000,   0x020000, CRC(69cf6b7c) SHA1(a7bdc62051d09636dcd54db102706a9b42465e63) ) // data
	ROM_RELOAD(         0x060000,   0x020000) // 128kb roms are mirrored
/*  Ciencias - General
    Espectaculos - Cine
    Sociales - Geografia Esp.
    Sociales - Historia  */
	ROM_LOAD( "05.bin", 0x080000,   0x020000, CRC(394cb674) SHA1(1390c666772f1e1e2da8866b960a3d24dc660e68) ) // questions
	ROM_RELOAD(         0x0a0000,   0x020000) // 128kb roms are mirrored
/*  Sociales - Geografia Mun.
    Varios - Cultura General */
	ROM_LOAD( "06.bin", 0x0c0000,   0x020000, CRC(aace7120) SHA1(5655b56a7c241bc7908081088042601174c0a0b2) ) // questions
	ROM_RELOAD(         0x0e0000,   0x020000) // 128kb roms are mirrored
/*  Deportes - General */
	ROM_LOAD( "07.bin", 0x100000,   0x020000, CRC(6618b002) SHA1(79942350da335a3362b6fc43527b6568ce134ceb) ) // questions
	ROM_RELOAD(         0x120000,   0x020000) // 128kb roms are mirrored
/*  Ciencias - General
    Varios - Cultura General */
	ROM_LOAD( "08.bin", 0x140000,   0x020000, CRC(6a4870dd) SHA1(f8ca94a5bc4ba3f512767901e4ae3579c2c6355a) ) // questions
	ROM_RELOAD(         0x160000,   0x020000) // 128kb roms are mirrored
	/*                  0x180000 to 0x1bffff EMPTY */
	/*                  0x1c0000 tt 0x1fffff EMPTY */
ROM_END

/* Is this actually official, or a hack? */
ROM_START( mastboyi )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hd647180.bin", 0x00000, 0x4000, CRC(75716dd1) SHA1(9b14b9b889b29b6022a3815de95487fb6a720d7a) ) // game code is internal to the CPU!
	ROM_LOAD( "3-mem-a.ic77", 0x04000, 0x4000, CRC(3ee33282) SHA1(26371e3bb436869461e9870409b69aa9fb1845d6) ) // sound data? (+ 1 piece of) 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(             0x04000, 0x4000 )
	ROM_CONTINUE(             0x04000, 0x4000 )
	ROM_CONTINUE(             0x04000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "gfx1", ROMREGION_ERASE00 ) /* RAM accessed by the video chip */
	/* 0x00000 - 0x0ffff = banked ram */

	ROM_REGION( 0x10000, "gfx2", ROMREGION_INVERT ) /* ROM accessed by the video chip */
	ROM_LOAD( "4.ic91", 0x00000, 0x10000, CRC(858d7b27) SHA1(b0ddf49df5665003f3616d67f7fc27408433483b) )

	ROM_REGION( 0x200000, "user1", 0 ) /* question data - 6 sockets */
	ROM_LOAD( "1-mem-c.ic75", 0x000000, 0x040000, CRC(7c7b1cc5) SHA1(73ad7bdb61d1f99ce09ef3a5a3ae0f1e72364eee) ) // 99% gfx
	ROM_LOAD( "2-mem-b.ic76", 0x040000, 0x020000, CRC(87015c18) SHA1(a16bf2707ce847da0923662796195b75719a6d77) ) // data
	ROM_RELOAD(               0x060000, 0x020000) // 128kb roms are mirrored
/*  Musica - Autori Canzoni
    Scienza - Natura
    Sport - Mondiali 90
    Tempo Libero - Hobby Giochi */
	ROM_LOAD( "5-rom.ic95",   0x080000, 0x020000, CRC(adc07f12) SHA1(2e0b46ac5884ad459bc354f56ff384ff1932f147) )
	ROM_RELOAD(               0x0a0000, 0x020000) // 128kb roms are mirrored
/*  Spettacolo - Cine-TV
    Sport - Generale  */
	ROM_LOAD( "6-rom.ic96",   0x0c0000, 0x020000, CRC(2c52cb1e) SHA1(d58f21c09bd3983497f74ab6c5a37977d9e30f0c) )
	ROM_RELOAD(               0x0e0000, 0x020000) // 128kb roms are mirrored
/*  Scienza - Geografia
    Scienza - Storia  */
	ROM_LOAD( "7-rom.ic97",   0x100000, 0x020000, CRC(7818408f) SHA1(2a69688b6cda5baf2a45966dd86f10b2fcd54b66) )
	ROM_RELOAD(               0x120000, 0x020000) // 128kb roms are mirrored
	/*                  0x140000 to 0x17ffff EMPTY */
	/*                  0x180000 to 0x1bffff EMPTY */
	/*                  0x1c0000 to 0x1fffff EMPTY */
ROM_END

static DRIVER_INIT( mastboy )
{
	mastboy_vram = memory_region( machine, "gfx1" ); // makes decoding the RAM based tiles easier this way
}

GAME( 1991, mastboy,  0,          mastboy, mastboy, mastboy, ROT0, "Gaelco", "Master Boy (Spanish, PCB Rev A)", 0 )
GAME( 1991, mastboyi, mastboy,    mastboy, mastboy, mastboy, ROT0, "Gaelco", "Master Boy (Italian, PCB Rev A)", 0 )
