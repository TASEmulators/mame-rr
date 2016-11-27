/***************************************************************************

    MAYGAY MV1 hardware

    preliminary driver by Phil Bennett

    Games supported:
        * Screen Play

    Other games on this hardware:
        * Believe it or not?
        * Caesar's Palace (reel to video)
        * Crossword Quiz
        * Give us a Clue
        * Special Effects (reel to video)
        * World Cup (reel to video)


Main MV1 board:

U1 ST 8901 TS68000CP12
U2 ST M74HC04B1 99135R
U3 ST 16S25HB1 9235 (think this is the gal chip)
U4 ST M74HC138B1 99131R
U5 P8948G MM74HC05N
U6 ST M74HC74B1 99144R
U7 ST M74HC08B1 99135R
U8 ST NE556N 99135
U9 ST M74HC259B1 99148R
U10 ST M74HC237B1 99148R
U11 ULN2803A 9135
U12 ST M74HC74B1 99144R
U13 ST M74HC32B1 99041R
U14 ST M74HCO4B1 99135R
U15 27C010 (Game Rom)
U16 27C010 (Game Rom)
U17 27C010 (Game Rom)
U18 27C010 (Game Rom)
U19 HYUNDAI HY6264ALP- 10 9147T
U20 HYUNDAI HY6264ALP- 10 9147T
U21 NEC IRELAND D8279C- 2 9135X8006]
U22 ULN2803A 9135
U23 ULN2803A 9135
U24 TEXAS INSTRUMENT F 9140 AN SN74HC148N
U25 ST 2 9148 EF68B21P
U26 Can't see one on the board!
U27 MOTOROLA MC68681P 14PT18715
U28 ST MC1488P 99136
U29 ST MC1489AP 99148
U30 ST HCF 4514 BE 2 9049
U31 MOTOROLA MC74F139N XXAA9145
U32 TEXAS INSTRUMENT TMS4464- 12NL IHE 9145
U33 TEXAS INSTRUMENT TMS4464- 12NL IHE 9114
U34 TEXAS INSTRUMENT TMS4464- 12NL IHE 9145
U35 TEXAS INSTRUMENT TMS4464- 12NL IHE 9114
U36 MHS S-82716-4 9210
U37 TEXAS INSTRUMENT TMS4464- 12NL IHE 9114
U38 TEXAS INSTRUMENT TMS4464- 12NL IHE 9145
U39 TEXAS INSTRUMENT TMS4464- 12NL IHE 9145
U40 TEXAS INSTRUMENT TMS4464- 12NL IHE 9114
U41 ST M74HC244B1 99131R
U42 ST M74HC244B1 99131R
U43 ST M74HC245B1 99136R
U44 ST M74HC245B1 99136R
U45 ST M74HC00B1 99135R
U46 ULN2803A 9135
U47 ULN2803A 9135
U48 YM2413 9127 HADG
U49 78L05 .194

The memory card that plugs in has 4 M27C010 game roms on.

The extra digital and reel board has the following:

U1 GS G06 KOREA GD74HC245
U2 SIEMENS SAB 8032B-P SINGAPORE BB INTEL 80 9148
U3 M27C512 (reels rom)
U4 ST M74HC373B1 99205R
U5 TEXAS INSTRUMENT 14530QT SN75155P
U6 ULN 2803A 9203
U7 ST M74HC374B1 99205R
U8 ST M74HC374B1 99205R
U9 ULN 2803A 9203
U10 ULN 2803A 9203
U11 ULN 2803A 9203
U12 M27C010 (Sound Rom)
U13 NEC JAPAN D7759C 9015KP009
U14 ST M74HC373B1 99205R
U15 Can't see one on the board!
U16 MOTOROLA MC74HC04AN FFA09202




upd7759 change:

* Only accept FIFO bytes when the chip is playing!!!

Port 1 is connected directly to the upd bus.
upd /CE is grounded
/WR bit is

Toggle reset pin
Write data on port 1
Toggle WR bit...

Then, it goes off to write.

INT1  Power failure?
INT2  -
INT3  V Sync
INT4  -
INT5  68681 DUART
INT6  -
INT7  -


DUART: 0 = Power failure

M68681 Output port drives slides

Todo:

Find lamps/reels after UPD changes.
***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/awpvid.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/6821pia.h"
#include "machine/68681.h"
#include "sound/2413intf.h"
#include "sound/upd7759.h"


/*************************************
 *
 *  Defines
 *
 *************************************/

#define DUART_CLOCK 	XTAL_3_6864MHz
#define PIXEL_CLOCK		0
#define MASTER_CLOCK	XTAL_16MHz
#define SOUND_CLOCK		XTAL_11_0592MHz

/*************************************
 *
 *  Video hardware
 *
 *************************************/

static int vsync_latch_preset;

#define VREG(a)		i82716.r[a]

enum
{
	VCR0 = 0,
	VCR1,
	RWBA,
	DWBA,
	DWSLM,
	DSBA,
	PAQ,
	ODTBA,
	ATBA,
	CTBA,
	CBASE,
	ATBAC,
	HVCONST0,
	HVCONST1,
	HVCONST2,
	HVCONST3
};

#define	VCR0_UCF			0x0001
#define VCR0_DEI			0x0002
#define VCR0_SAB			0x0004
#define VCR0_DEN			0x0008
#define VCR0_HRS			0x0010
#define VCR0_DOF			0x0020

#define VCR0_DS_MASK		0x00c0
#define VCR0_DS_SHIFT       6
#define VCR0_BLINK_MASK		0x1f00
#define VCR0_BLINK_SHIFT    8
#define VCR0_DUTY_MASK		0xe000
#define VCR0_DUTY_SHIFT     13

static const UINT32 banks[4] = { 0, 0x40000/2, 0x20000/2, 0x60000/2 };

static struct
{
	running_device *duart68681;
} maygayv1_devices;

#define DRAM_BANK_SEL		(banks[(VREG(DSBA) >> 7) & 3])

static struct
{
	UINT16	r[16];
	UINT16	*dram;

	UINT8	*line_buf;	// there's actually two
} i82716;

static WRITE16_HANDLER( i82716_w )
{
	// Accessing register window?
	if ((VREG(RWBA) & 0xfff0) == (offset & 0xfff0))
	{
		// Register segment is fixed at start of DRAM
		COMBINE_DATA(&i82716.dram[offset & 0xf]);
	}

	// Accessing data window?
	// TODO: mask
	if (offset >= (VREG(DWBA) & 0xf800))
	{
		offset -= (VREG(DWBA) & 0xf800);
		COMBINE_DATA(&i82716.dram[DRAM_BANK_SEL + (VREG(DSBA) & 0xf800) + offset]);
	}
}

static READ16_HANDLER( i82716_r )
{
	// Accessing register window?
	if ((VREG(RWBA) & ~0xf) == (offset & ~0xf))
	{
		return(i82716.r[offset & 0xf]);
	}

	// Accessing data window? TODO: mask?
	if (VREG(VCR1) & 4)
	{
		if (offset >= (VREG(DWBA) & 0xf800))
		{
			offset -= (VREG(DWBA) & 0xf800);
			return i82716.dram[DRAM_BANK_SEL +(VREG(DSBA) & 0xf800) + (offset)];
		}
	}

	return 0;
}

static VIDEO_START( maygayv1 )
{

}


static VIDEO_UPDATE( maygayv1 )
{
	UINT16 *atable = &i82716.dram[VREG(ATBA)];
	UINT16 *otable = &i82716.dram[VREG(ODTBA) & 0xfc00];  // both must be bank 0

    int sl, sx;
	int slmask = 0xffff;     // TODO: Save if using scanline callbacks
	int xbound = (VREG(DWBA) & 0x3f8) | 7;

	/* Sign extend to 10 bits */
	xbound = (xbound & 0x3ff) - (xbound & 0x400);

	/* If screen output is disabled, fill with black */
	if (!(VREG(VCR0) & VCR0_DEN))
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	/* For every scanline... */
	for (sl = cliprect->min_x; sl <= cliprect->max_y; ++sl)
	{
		int obj;
		UINT16 aflags = atable[sl];
		UINT16 slmask_old = slmask;

		UINT16 *bmp_ptr = BITMAP_ADDR16(bitmap, sl, 0);

		slmask = 0xffff ^ (slmask ^ aflags);

		/* Clear the frame buffer on each line to BG colour (palette entry 2) */
		/* 4bpp only ! */
		memset(i82716.line_buf, 0x22, 512);

    	/* Parse the list of 16 objects */
		for (obj = 0; obj < 16; ++obj)
		{
			int offs = obj * 4;

			// Draw on this line?
			if ( !BIT(slmask, obj) )
            {
				UINT32	objbase, trans, width, res, cspec;
				INT32	x, xpos;
				UINT16	w0, w1, w2;
				UINT16	*objptr;
				UINT8 *bmpptr; // ?

				/* Get object table entry words */
				w0 = otable[offs];
                w1 = otable[offs + 1];
				w2 = otable[offs + 2];

				/* Blanked */
				if ( BIT(w0, 4) )
					break;

//              if ( BIT(w0, 5) )
//                  printf("Blinking\n");

				/* Resolution: either 4bpp or 2bpp */
				res = (w0 >> 9) & 3;


				/* Expand 2bpp to 3bpp */
				cspec = (w0 & 3) << 2;

				if ( BIT(w0, 11) )
				{
					logerror("i82716: Characters not supported\n");
					break;
				}

				/* 10-bit signed - in terms of bytes */
				xpos = (w1 & 0x3ff);
				xpos = (xpos & 0x3ff) - (xpos & 0x400);

				/* Transparency detect enable */
				trans = BIT(w0, 2);

				/* Width is expressed in units of 64-bit words */
				width = (w1 >> 10) & 0x3f;

				/* First scanline? Clear current object entry address */
				if ( BIT(slmask_old, obj) )
                	otable[offs + 3] = 0;

				/* Bitmap data pointer */
				objbase = ((w0 & 0x00c0) << 10) | w2;
				objptr = &i82716.dram[objbase + ((4 * width) * otable[offs + 3])];

				// endian alert
                bmpptr = (UINT8*)objptr;

				// 4bpp
				for (x = xpos; x < MIN(xbound, xpos + width * 8); ++x)
				{
					if (x >= 0)
					{
						UINT8 p1 = *bmpptr & 0xf;
						UINT8 p2 = *bmpptr >> 4;

						if (!trans || p1)
					        i82716.line_buf[x] = p1;

						if (!trans || p2)
					        i82716.line_buf[x] |= p2 << 4;
					}
					bmpptr++;
				}

				/* Update scanline pointer - WRONG */
				otable[offs + 3]++;
			}
		}

		// Write it out
		for (sx = cliprect->min_x; sx < cliprect->max_x; sx += 2)
		{
			UINT8 pix = i82716.line_buf[sx / 2];

			bmp_ptr[sx + 0] = pix & 0xf;
			bmp_ptr[sx + 1] = pix >> 4;
		}
	}

	return 0;
}

static VIDEO_EOF( maygayv1 )
{
	// UCF
	if (VREG(VCR0) & VCR0_UCF)
	{
		int i;

		for (i = 0; i < 16; ++i)
			VREG(i) = i82716.dram[i];
	}
	else
	{
    	VREG(VCR0) = i82716.dram[VCR0];
    	VREG(ATBA) = i82716.dram[ATBA];
	}

	if (!(VREG(VCR0) & VCR0_DEI))
	{
		int i;
		UINT16 *palbase = &i82716.dram[VREG(CTBA)];

		for (i = 0; i < 16; ++i)
		{
			UINT16 entry = *palbase++;
			palette_set_color_rgb(machine, entry & 0xf, pal4bit(entry >> 12), pal4bit(entry >> 8), pal4bit(entry >> 4));
		}
	}
}



/*************************************
 *
 *  68000 CPU memory handlers
 *
 *************************************/

/*
    68681
    YM2413
    68B21
    8279C

    8a0008 0xe0
    8a000c 0x7
    8a000e 0x33
    8a000a 0x8
    8a001c 0xff R/W
*/



static WRITE16_HANDLER( write_odd )
{
}

//;860008 is a latch of some sort
static READ16_HANDLER( read_odd )
{
	return 0;
}


static struct _i8279_state
{
	UINT8	command;
	UINT8	mode;
	UINT8	prescale;
	UINT8	inhibit;
	UINT8	clear;
	UINT8	fifo[8];
	UINT8	ram[16];
} i8279;

/* TODO */
static void update_outputs(UINT16 which)
{
	int i;

	/* update the items in the bitmask */
	for (i = 0; i < 16; i++)
		if (which & (1 << i))
		{
/*
            int val;

            val = i8279.ram[i] & 0xff;

            val = i8279.ram[i] & 0x0f;
            if (i8279.inhibit & 0x01)
                val = i8279.clear & 0x0f;

                if(val) printf("%x\n", val);

            val = i8279.ram[i] >> 4;
            if (i8279.inhibit & 0x02)
                val = i8279.clear >> 4;

                if(val) printf("%x\n", val);
*/
		}
}

static READ16_HANDLER( maygay_8279_r )
{
	static const char *const portnames[] = { "STROBE1","STROBE2","STROBE3","STROBE4","STROBE5","STROBE6","STROBE7","STROBE8" };
	UINT8 result = 0xff;
	UINT8 addr;

	/* read data */
	if ((offset & 1) == 0)
	{
		switch (i8279.command & 0xe0)
		{
			/* read sensor RAM */
			case 0x40:
				addr = i8279.command & 0x07;

				result = input_port_read(space->machine, portnames[addr]);

				/* handle autoincrement */
				if (i8279.command & 0x10)
					i8279.command = (i8279.command & 0xf0) | ((addr + 1) & 0x0f);

				break;

			/* read display RAM */
			case 0x60:

				/* set the value of the corresponding outputs */
				addr = i8279.command & 0x0f;
				result = i8279.ram[addr];

				/* handle autoincrement */
				if (i8279.command & 0x10)
					i8279.command = (i8279.command & 0xf0) | ((addr + 1) & 0x0f);
				break;
		}
	}
	/* read status word */
	else
	{
		printf("read 0xfc%02x\n", offset);
		result = 0x10;
	}
	return result;
}


static WRITE16_HANDLER( maygay_8279_w )
{
	UINT8 addr;

	data >>= 8;

	/* write data */
	if ((offset & 1) == 0)
	{
		switch (i8279.command & 0xe0)
		{
			/* write display RAM */
			case 0x80:

				/* set the value of the corresponding outputs */
				addr = i8279.command & 0x0f;
				if (!(i8279.inhibit & 0x04))
					i8279.ram[addr] = (i8279.ram[addr] & 0xf0) | (data & 0x0f);
				if (!(i8279.inhibit & 0x08))
					i8279.ram[addr] = (i8279.ram[addr] & 0x0f) | (data & 0xf0);
				update_outputs(1 << addr);

				/* handle autoincrement */
				if (i8279.command & 0x10)
					i8279.command = (i8279.command & 0xf0) | ((addr + 1) & 0x0f);
				break;
		}
	}

	/* write command */
	else
	{
		i8279.command = data;

		switch (data & 0xe0)
		{
			/* command 0: set mode */
			/*
                Display modes:

                00 = 8 x 8-bit character display -- left entry
                01 = 16 x 8-bit character display -- left entry
                10 = 8 x 8-bit character display -- right entry
                11 = 16 x 8-bit character display -- right entry

                Keyboard modes:

                000 = Encoded scan keyboard -- 2 key lockout
                001 = Decoded scan keyboard -- 2 key lockout
                010 = Encoded scan keyboard -- N-key rollover
                011 = Decoded scan keyboard -- N-key rollover
                100 = Encoded scan sensor matrix
                101 = Decoded scan sensor matrix
                110 = Strobed input, encoded display scan
                111 = Strobed input, decoded display scan
            */
			case 0x00:
				logerror("8279: display mode = %d, keyboard mode = %d\n", (data >> 3) & 3, data & 7);
				i8279.mode = data & 0x1f;
				break;

			/* command 1: program clock */
			case 0x20:
				logerror("8279: clock prescaler set to %02X\n", data & 0x1f);
				i8279.prescale = data & 0x1f;
				break;

			/* command 2: read FIFO/sensor RAM */
			/* command 3: read display RAM */
			/* command 4: write display RAM */
			case 0x40:
			case 0x60:
			case 0x80:
				break;

			/* command 5: display write inhibit/blanking */
			case 0xa0:
				i8279.inhibit = data & 0x0f;
				update_outputs(~0);
				logerror("8279: clock prescaler set to %02X\n", data & 0x1f);
				break;

			/* command 6: clear */
			case 0xc0:
				i8279.clear = (data & 0x08) ? ((data & 0x04) ? 0xff : 0x20) : 0x00;
				if (data & 0x11)
					memset(i8279.ram, i8279.clear, sizeof(i8279.ram));
				break;

			/* command 7: end interrupt/error mode set */
			case 0xe0:
				break;
		}
	}
}






static WRITE16_HANDLER( vsync_int_ctrl )
{
	vsync_latch_preset = data & 0x0100;

	// Active low
	if (!(vsync_latch_preset))
		cputag_set_input_line(space->machine, "maincpu", 3, CLEAR_LINE);
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x100000, 0x17ffff) AM_ROM AM_REGION("maincpu", 0x80000)
	AM_RANGE(0x820000, 0x820003) AM_READWRITE(maygay_8279_r, maygay_8279_w)
	AM_RANGE(0x800000, 0x800003) AM_DEVWRITE8( "ymsnd", ym2413_w, 0xff00 )
	AM_RANGE(0x860000, 0x86000d) AM_READWRITE(read_odd, write_odd)
	AM_RANGE(0x86000e, 0x86000f) AM_WRITE(vsync_int_ctrl)
	AM_RANGE(0x880000, 0x89ffff) AM_READWRITE(i82716_r, i82716_w)
	AM_RANGE(0x8a0000, 0x8a001f) AM_DEVREADWRITE8( "duart68681", duart68681_r, duart68681_w, 0xff)
	AM_RANGE(0x8c0000, 0x8c000f) AM_DEVREADWRITE8("pia", pia6821_r, pia6821_w, 0xff)
ADDRESS_MAP_END


/*************************************
 *
 *  8032 CPU memory handlers
 *
 *************************************/

/*
 74HC245 @ U1 read port
 (P3.4 = /ENABLE)
 P1.0 = Reel 1 optic (I)
 P1.1 = Reel 2 optic (I)
 P1.2 = Reel 3 optic (I)
 P1.3
 P1.4 = DIPSWITCH (GND/ON by default)
 P1.5 = DIPSWITCH (GND/ON by default)
 P1.6 = DIPSWITCH (GND/ON by default)
 P1.7 = DIPSWITCH (GND/ON by default)

 WRITE
 P1.0 - 7 => D7759C
          => 74HC374 @ U8 write port (CLK = 3.7)

 P3.0 = RXD
 P3.1 = TXD
 P3.2 = /UPD_RESET
 P3.3 = !(/UPD_BUSY)
 P3.4 = U1 /ENABLE
 P3.5 = Status LED (inverted twice)
 P3.6 = (WR) /UPD_START
 P3.7 = (RD) U8 CLK - for writing! P1



*/
static UINT8 p1; // save state
static UINT8 p3; // save state

static READ8_HANDLER( mcu_r )
{
	switch (offset)
	{
		case 1:
		{
			if ( !BIT(p3, 4) )
				return (input_port_read(space->machine, "REEL"));	// Reels???
			else
				return 0;
		}

		case 3: return upd7759_busy_r(0) ? 0 : 0x08;
	}
	return 0;
}

static WRITE8_HANDLER( mcu_w )
{
			logerror("O %x D %x",offset,data);

	switch (offset)
	{
		// Bottom nibble = UPD
		case 1:
			p1 = data;
//          upd7759_msg_w(0, data);//?
			break;
		case 3:
			upd7759_reset_w (0, BIT(data, 2));
			upd7759_start_w(0, BIT(data, 6));

//          if ( !BIT(p3, 7) && BIT(data, 7) )
				// P1 propagates to outputs

			p3 = data;
			break;
	}
}


static ADDRESS_MAP_START( sound_prg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_data, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x0000, 0xffff) AM_RAM // nothing?
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READWRITE(mcu_r, mcu_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( screenpl )
	PORT_START("STROBE1")
	PORT_DIPNAME( 0x01, 0x01, "DSW01")
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW02")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW03")
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW04")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Stake selection?")
	PORT_DIPSETTING(	0x10, "5p" )
	PORT_DIPSETTING(	0x00, "10p" )
	PORT_DIPNAME( 0x20, 0x20, "DSW06")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW07")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW08")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("STROBE2")
	PORT_DIPNAME( 0x01, 0x01, "Teste")
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Unk Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Nudge 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Nudge 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Nudge 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("?")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Collect")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("?")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Spin")

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Red")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Yellow")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Green")
	PORT_DIPNAME( 0x08, 0x08, "DSW34")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW35")
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Jackpot Selection")
	PORT_DIPSETTING(	0x20, "600p" )
	PORT_DIPSETTING(	0x00, "300p" )
	PORT_DIPNAME( 0x40, 0x40, "Reset?")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Cash door")
	PORT_DIPSETTING(	0x80, "Open"   )
	PORT_DIPSETTING(	0x00, "Closed" )

	PORT_START("STROBE5")
	PORT_DIPNAME( 0x01, 0x01, "DSW41")
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Re-fill key")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW43")
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW44")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW45")
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW46")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW47")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW48")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("STROBE6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Token")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("100p")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("20p")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("10p")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("REEL")
	PORT_DIPNAME( 0x01, 0x00, "REEL 1")
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "REEL 2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "REEL 3")
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "REEL 4")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "MCU DIP1")
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "MCU DIP2")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "MCU DIP3")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "MCU DIP4")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

/***************************************************************************

    68681 DUART

***************************************************************************/

static void duart_irq_handler(running_device *device, UINT8 vector)
{
	cputag_set_input_line_and_vector(device->machine, "maincpu", 5, ASSERT_LINE, vector);
//  cputag_set_input_line(device->machine, "maincpu", 5, state ? ASSERT_LINE : CLEAR_LINE);
};

static int d68681_val;

static void duart_tx(running_device *device, int channel, UINT8 data)
{
	if (channel == 0)
	{
		d68681_val = data;
		cputag_set_input_line(device->machine, "soundcpu", MCS51_RX_LINE, ASSERT_LINE);  // ?
	}

};

static const duart68681_config maygayv1_duart68681_config =
{
	duart_irq_handler,
	duart_tx,
	NULL,
	NULL
};


static int data_to_i8031(running_device *device)
{
	return d68681_val;
}

static void data_from_i8031(running_device *device, int data)
{
	duart68681_rx_data(maygayv1_devices.duart68681, 0, data);
}

static READ8_DEVICE_HANDLER( b_read )
{
	// Meters - upper nibble?
	return 0xff;
}

static WRITE8_DEVICE_HANDLER( b_writ )
{
	logerror("B WRITE %x\n",data);
}


/* U25 ST 2 9148 EF68B21P */
static const pia6821_interface pia_intf =
{
	DEVCB_HANDLER(b_read),		/* port A in */
	DEVCB_HANDLER(b_read),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(b_writ),		/* port A out */
	DEVCB_HANDLER(b_writ),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};


static MACHINE_START( maygayv1 )
{
	i82716.dram = auto_alloc_array(machine, UINT16, 0x80000/2);   // ???
	i82716.line_buf = auto_alloc_array(machine, UINT8, 512);

	state_save_register_global_pointer(machine, i82716.dram, 0x40000);

//  duart_68681_init(DUART_CLOCK, duart_irq_handler, duart_tx);

	i8051_set_serial_tx_callback(machine->device("soundcpu"), data_from_i8031);
	i8051_set_serial_rx_callback(machine->device("soundcpu"), data_to_i8031);
}

static MACHINE_RESET( maygayv1 )
{
	// ?
	maygayv1_devices.duart68681 = machine->device( "duart68681" );
	memset(i82716.dram, 0, 0x40000);
	i82716.r[RWBA] = 0x0200;
}


static INTERRUPT_GEN( vsync_interrupt )
{
	if (vsync_latch_preset)
		cputag_set_input_line(device->machine, "maincpu", 3, ASSERT_LINE);
}


static MACHINE_DRIVER_START( maygayv1 )
	MDRV_CPU_ADD("maincpu", M68000, MASTER_CLOCK / 2)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", vsync_interrupt)

	MDRV_CPU_ADD("soundcpu", I8052, SOUND_CLOCK)
	MDRV_CPU_PROGRAM_MAP(sound_prg)
	MDRV_CPU_DATA_MAP(sound_data)
	MDRV_CPU_IO_MAP(sound_io)

	MDRV_PIA6821_ADD("pia", pia_intf)

	MDRV_MACHINE_START(maygayv1)
	MDRV_MACHINE_RESET(maygayv1)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* TODO: Use real video timings */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 300)
	MDRV_SCREEN_VISIBLE_AREA(0, 640 - 1, 0, 300 - 1)

	MDRV_PALETTE_LENGTH(16)

	MDRV_DUART68681_ADD("duart68681", DUART_CLOCK, maygayv1_duart68681_config)

	MDRV_VIDEO_START(maygayv1)
	MDRV_VIDEO_UPDATE(maygayv1)
	MDRV_VIDEO_EOF(maygayv1)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd",YM2413, MASTER_CLOCK / 4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.8)

	MDRV_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( screenpl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20p6pnd_u15.bin", 0x00000, 0x20000, CRC(4334453c) SHA1(5c18acf29c7b3f44589b92d74b79ac66730db810) )
	ROM_LOAD16_BYTE( "20p6pnd_u16.bin", 0x00001, 0x20000, CRC(90b3f67c) SHA1(a58a0bc4ccccf083fe3222f02eb06ee5fa6f386a) )
	ROM_LOAD16_BYTE( "20p6pnd_u17.bin", 0x40000, 0x20000, CRC(ba576b11) SHA1(3ba7bcaf4e3cc4eaeeece6e3f4957c3a8dfd5752) )
	ROM_LOAD16_BYTE( "20p6pnd_u18.bin", 0x40001, 0x20000, CRC(24dd1aff) SHA1(833c59e5b75130a8dc3a63027e09c0f5c7ed17f5) )

	ROM_LOAD16_BYTE( "20p6pnd_u2.bin",  0x80000, 0x20000, CRC(ee51ed98) SHA1(262e773cdb1465983a8f931698bc73de7c324088) )
	ROM_LOAD16_BYTE( "20p6pnd_u1.bin",  0x80001, 0x20000, CRC(d57bbe69) SHA1(b7cd93cef4828418328ca4ff16c496da7b2065e2) )
	ROM_LOAD16_BYTE( "20p6pnd_u4.bin",  0xc0000, 0x20000, CRC(01aafd7e) SHA1(d2161066655218468da8eae8aa9da8a80c07c489) )
	ROM_LOAD16_BYTE( "20p6pnd_u3.bin",  0xc0001, 0x20000, CRC(aa02dc54) SHA1(a05c8c26480f3bae671428380c7684b9c29b5a53) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "reels.bin", 0x0000, 0x10000, CRC(1319cf82) SHA1(7a233072890361bcf384de4f90170c2ca713b1de) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "sound.bin", 0x00000, 0x20000, CRC(498dd74f) SHA1(80bb204b3e9cadcecbfa75c78c52fb9908566c5e) )
ROM_END

ROM_START( screenp1 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sa4-379.u15", 0x00000, 0x20000, CRC(e21b120d) SHA1(bcdebf797381b0c585dbd3511b5e984f22de2206) )
	ROM_LOAD16_BYTE( "sa4-379.u16", 0x00001, 0x20000, CRC(b04588b7) SHA1(1f9b933e441969c95bbbabbfbe44349bd945c326) )
	ROM_LOAD16_BYTE( "sa4-379.u17", 0x40000, 0x20000, CRC(4b6cdc43) SHA1(1d6a4796ce67d0d00fe74a6bafd8b731450cdaab) )
	ROM_LOAD16_BYTE( "sa4-379.u18", 0x40001, 0x20000, CRC(d986355f) SHA1(86d3f1712cd1bcc90a54945a2baccae2596de691) )

	ROM_LOAD16_BYTE( "sq3-458.u2",  0x80000, 0x20000, CRC(7091dfcd) SHA1(d28abd70db5c49baa93f0488e443f29c27a7a559) )
	ROM_LOAD16_BYTE( "sq3-458.u1",  0x80001, 0x20000, CRC(1bb0efbf) SHA1(59d7e2e51928df149764502bc4bd5736463f40d7) )
	ROM_LOAD16_BYTE( "sq3-458.u4",  0xc0000, 0x20000, CRC(0fb0fc84) SHA1(e7ef68130f9627a842849f41f67accf8593a0819) )
	ROM_LOAD16_BYTE( "sq3-458.u3",  0xc0001, 0x20000, CRC(ef4617d8) SHA1(48231405a775585451bf970db5bb57ec2f238250) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sr2-002", 0x0000, 0x10000, CRC(1319cf82) SHA1(7a233072890361bcf384de4f90170c2ca713b1de) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "dig2-001.u12", 0x00000, 0x20000, CRC(498dd74f) SHA1(80bb204b3e9cadcecbfa75c78c52fb9908566c5e) )
ROM_END

ROM_START( screenp2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sa4-280.u15", 0x00000, 0x20000, CRC(d7206438) SHA1(70e7dc7d58bfa7dfe00250ab46fa71e951dbf901) )
	ROM_LOAD16_BYTE( "sa4-280.u16", 0x00001, 0x20000, CRC(f99e972f) SHA1(b01c8796967ff7f27269b31ef983b5fb26b03aab) )
	ROM_LOAD16_BYTE( "sa4-280.u17", 0x40000, 0x20000, CRC(cbde5343) SHA1(e341d642d8537bc221b3ca9803c221dc0cdf86c3) )
	ROM_LOAD16_BYTE( "sa4-280.u18", 0x40001, 0x20000, CRC(885b887b) SHA1(9cfb145c8cca49450fabbf4efab9c70f98ecd2af) )

	ROM_LOAD16_BYTE( "u2.bin", 0x80000, 0x20000, CRC(7091dfcd) SHA1(d28abd70db5c49baa93f0488e443f29c27a7a559) )
	ROM_LOAD16_BYTE( "u1.bin", 0x80001, 0x20000, CRC(1bb0efbf) SHA1(59d7e2e51928df149764502bc4bd5736463f40d7) )
	ROM_LOAD16_BYTE( "u4.bin", 0xc0000, 0x20000, CRC(0fb0fc84) SHA1(e7ef68130f9627a842849f41f67accf8593a0819) )
	ROM_LOAD16_BYTE( "u3.bin", 0xc0001, 0x20000, CRC(ef4617d8) SHA1(48231405a775585451bf970db5bb57ec2f238250) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "reels.bin", 0x00000, 0x10000, CRC(1319cf82) SHA1(7a233072890361bcf384de4f90170c2ca713b1de) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "dig2-001.u12", 0x00000, 0x20000, CRC(498dd74f) SHA1(80bb204b3e9cadcecbfa75c78c52fb9908566c5e) )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

static DRIVER_INIT( screenpl )
{
	p1 = p3 = 0xff;
}

GAME( 1991, screenpl, 0,        maygayv1, screenpl, screenpl, ROT0, "Maygay", "Screen Play (ver. 4.0)",               GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_REQUIRES_ARTWORK )
GAME( 1991, screenp1, screenpl, maygayv1, screenpl, screenpl, ROT0, "Maygay", "Screen Play (ver. 1.9)",               GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_REQUIRES_ARTWORK )
GAME( 1991, screenp2, screenpl, maygayv1, screenpl, screenpl, ROT0, "Maygay", "Screen Play (ver. 1.9, Isle of Man)",  GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_REQUIRES_ARTWORK )
