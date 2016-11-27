/******************************************************************************

    Bell-Fruit Cobra I/II and Viper Hardware

    driver by Phil Bennett and Anonymous

    Games supported:
        * A Question of Sport [2 sets]
        * Beeline (non-working - missing disk)
        * Every Second Counts
        * Inquizitor (Viper hardware, non-working - missing disk)
        * Quizvaders
        * Treble Top

    Other games on this hardware:
        * Brain Box
        * Quintoon alt. version (Cobra II/Cyclone hardware)

    Notes:

    The hardware is based on a chipset known as 'Flare One', developed
    by Flare Technology. It consists of a 16-bit DSP (intended for sound
    synthesis and 3D maths), an 8bpp blitter and a video controller, driven
    by a Z80.

    Flare One would evolve to become 'Slipstream', used by the unreleased
    Konix Multisystem console.

    The Flare One chipset is implemented as four Texas Instruments ASICs,
    each an 84 pin PLCC package:

    CF30204, CF30205, CF30206 (DSP) and CF30207.

    The hardware using this chipset is as follows:

    Viper
    =====

    A video expansion PCB for Scorpion I?
    On some PCB revisions there is audio output circuitry connected to the DSP.
    Viper uses a WD1772 type floppy disk controller.

    Cobra I
    =======

    A combination of Viper and Scorpion I hardware on a single PCB.
    Cobra uses an NEC '765 type FDC. Later revisions have no DSP.

    Cobra II (Cyclone)
    ==================

    A compact video expansion board for Scorpion II.
    The Z80 is replaced by a Z180 and there is no Flare DSP or FDC


    To do:

    * Complete blitter emulation
    * Cobra II support
    * Hook up additional inputs, EM meters, lamps etc

    Known issues:

    * All games bar qos: NVRAM not saved

    * Viper does not have a colour palette - the Flare chipset drives RGB direct.
      To fix this I set default values in the palette when the machine is initialised
    * CPU execution rate is wrong, the hardware adds 1 TCycle to each access which is unaccounted for.
    * Plane priority is probably wrong but it's only used in Treble Top.
    * Blitter loop counts and step are wrong - they are 9 bit counts, not 8.
    * Blitter emulation doesn't support hi-res mode (needed for Inquizitor)

******************************************************************************/

#include "emu.h"
#include "machine/6850acia.h"
#include "machine/meters.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "sound/upd7759.h"
#include "sound/ay8910.h"

/*
    Defines
*/
#define Z80_XTAL	5910000		/* Unconfirmed */
#define M6809_XTAL	1000000


/*
    Globals
*/
static UINT8 bank_data[4];
static UINT8 *work_ram;
static UINT8 *video_ram;
static UINT8 h_scroll;
static UINT8 v_scroll;
static UINT8 flip_8;
static UINT8 flip_22;
static UINT8 videomode;

/* UART source/sinks */
static UINT8 z80_m6809_line;
static UINT8 m6809_z80_line;
static UINT8 data_r;
static UINT8 data_t;

static int irq_state;
static int acia_irq;
static int vblank_irq;
static int blitter_irq;

static UINT8 z80_int;
static UINT8 z80_inten;

/* EM and lamps stuff */
static UINT32 meter_latch;
static UINT32 mux_input;
static UINT32 mux_outputlatch;

/*
    Function prototypes
*/
INLINE void z80_bank(running_machine *machine, int num, int data);


static void update_irqs(running_machine *machine)
{
	int newstate = blitter_irq || vblank_irq || acia_irq;

	if (newstate != irq_state)
	{
		irq_state = newstate;
		cputag_set_input_line(machine, "maincpu", 0, irq_state ? ASSERT_LINE : CLEAR_LINE);
	}
}

/***************************************************************************

    Split into video\cobra.c !

***************************************************************************/

typedef union
{
#ifdef LSB_FIRST
	struct { UINT16 loword, hiword ; } ;
	struct { UINT8 addr0, addr1, addr2; };
#else
	struct { UINT16 hiword, loword ; } ;
	struct { UINT8 addr2, addr1, addr0; };
#endif
	UINT32 addr;
} ADDR_REG;

/* Blitter register flag bits */
#define	CMD_RUN			0x01
#define CMD_COLST		0x02
#define CMD_PARRD		0x04		/* Never used? */
#define CMD_SRCUP		0x08
#define CMD_DSTUP		0x10
#define CMD_LT0 		0x20
#define CMD_LT1			0x40
#define CMD_LINEDRAW	0x80


/* All unconfirmed */
//#define SRCDST_CMP    0x10
//#define SRCDST_WRAP   0x20
//#define SRCDST_SIGN   0x40
#define SRCDST_A_1		0x80		/* This might be correct for line drawing? */

/* These appear to be correct */
#define MODE_SSIGN		0x80
#define MODE_DSIGN		0x40
#define MODE_YFRAC		0x20
#define MODE_BITTOBYTE	0x04
#define MODE_PALREMAP	0x10

#define CMPFUNC_LT  	0x01
#define CMPFUNC_EQ  	0x02
#define CMPFUNC_GT  	0x04
#define CMPFUNC_BEQ		0x08
#define CMPFUNC_LOG0	0x10
#define CMPFUNC_LOG1	0x20
#define CMPFUNC_LOG2	0x40
#define CMPFUNC_LOG3	0x80

/*
    Blitter state
*/
static struct
{
	ADDR_REG	program;

	UINT8		control;
	UINT8		status;

	UINT8		command;
	ADDR_REG	source;
	ADDR_REG	dest;
	UINT8		modectl;
	UINT8		compfunc;
	UINT8		outercnt;

	UINT8		innercnt;
	UINT8		step;
	UINT8		pattern;
} blitter;

#define LOOPTYPE ( ( blitter.command&0x60 ) >> 5 )

/*
    MUSIC Semiconductor TR9C1710 RAMDAC or equivalent
*/
static struct
{
	UINT8	addr_w;
	UINT8	addr_r;
	UINT8	mask;

	/* 18-bit colors */
	UINT8	color_r[3];
	UINT8	color_w[3];
	UINT32	table[256];

	/* Access counts */
	UINT8	count_r;
	UINT8	count_w;
} ramdac;


#define BLUE_0 0
#define BLUE_1 1
#define BLUE_2 2
#define BLUE_3 3
#define GREEN_0 ( 0 << 2 )
#define GREEN_1 ( 1 << 2 )
#define GREEN_2 ( 2 << 2 )
#define GREEN_3 ( 3 << 2 )
#define GREEN_4 ( 4 << 2 )
#define GREEN_5 ( 5 << 2 )
#define GREEN_6 ( 6 << 2 )
#define GREEN_7 ( 7 << 2 )
#define RED_0 ( 0 << 5 )
#define RED_1 ( 1 << 5 )
#define RED_2 ( 2 << 5 )
#define RED_3 ( 3 << 5 )
#define RED_4 ( 4 << 5 )
#define RED_5 ( 5 << 5 )
#define RED_6 ( 6 << 5 )
#define RED_7 ( 7 << 5 )

static const UINT8 col4bit_default[16]=
{
	BLUE_0 | GREEN_0 | RED_0,
	BLUE_1,
	GREEN_2,
	BLUE_1 | GREEN_2,
	RED_2,
	RED_2 | BLUE_1,
	RED_2 | GREEN_2,
	RED_2 | GREEN_2 | BLUE_1,
	BLUE_2 | GREEN_5 | RED_5,
	BLUE_3,
	GREEN_7,
	BLUE_3 | GREEN_7,
	RED_7,
	RED_7 | BLUE_3,
	RED_7 | GREEN_7,
	RED_7 | GREEN_7 | BLUE_3
};

static const UINT8 col3bit_default[16]=
{
	0,
	BLUE_3,
	GREEN_7,
	BLUE_3 | GREEN_7,
	RED_7,
	RED_7 | BLUE_3,
	RED_7 | GREEN_7,
	RED_7 | GREEN_7 | BLUE_3,
	0,
	BLUE_3,
	GREEN_7,
	BLUE_3 | GREEN_7,
	RED_7,
	RED_7 | BLUE_3,
	RED_7 | GREEN_7,
	RED_7 | GREEN_7 | BLUE_3
};

static UINT8 col4bit[16];
static UINT8 col3bit[16];
static UINT8 col8bit[256];
static UINT8 col7bit[256];
static UINT8 col6bit[256];
static const UINT8 col76index[] = {0, 2, 4, 7};


static VIDEO_START( bfcobra )
{
	int i;

	memcpy(col4bit, col4bit_default, sizeof(col4bit));
	memcpy(col3bit, col3bit_default, sizeof(col3bit));
	for (i = 0; i < 256; ++i)
	{
		UINT8 col;

		col8bit[i] = i;
		col = i & 0x7f;
		col = (col & 0x1f) | (col76index[ ( (col & 0x60) >> 5 ) & 3] << 5);
		col7bit[i] = col;

		col = (col & 3) | (col76index[( (col & 0x0c) >> 2) & 3] << 2 ) |
			  (col76index[( (col & 0x30) >> 4) & 3] << 5 );
		col6bit[i] = col;
	}
}

static VIDEO_UPDATE( bfcobra )
{
	int x, y;
	UINT8  *src;
	UINT32 *dest;
	UINT32 offset;
	UINT8 *hirescol;
	UINT8 *lorescol;

	/* Select screen has to be programmed into two registers */
	/* No idea what happens if the registers are different */
	if (flip_8 & 0x40 && flip_22 & 0x40)
		offset = 0x10000;
	else
		offset = 0;

	if(videomode & 0x20)
	{
		hirescol = col3bit;
		lorescol = col7bit;
	}
	else if(videomode & 0x40)
	{
		hirescol = col4bit;
		lorescol = col6bit;
	}
	else
	{
		hirescol = col4bit;
		lorescol = col8bit;
	}

	for (y = cliprect->min_y; y <= cliprect->max_y; ++y)
	{
		UINT16 y_offset = (y + v_scroll) * 256;
		src = &video_ram[offset + y_offset];
		dest = BITMAP_ADDR32(bitmap, y, 0);

		for (x = cliprect->min_x; x <= cliprect->max_x / 2; ++x)
		{
			UINT8 x_offset = x + h_scroll;
			UINT8 pen = *(src + x_offset);

			if ( ( videomode & 0x81 ) == 1 || (videomode & 0x80 && pen & 0x80) )
			{
				*dest++ = screen->machine->pens[hirescol[pen & 0x0f]];
				*dest++ = screen->machine->pens[hirescol[(pen >> 4) & 0x0f]];
			}
			else
			{
				*dest++ = screen->machine->pens[lorescol[pen]];
				*dest++ = screen->machine->pens[lorescol[pen]];
			}
		}
	}

	return 0;
}

INLINE UINT8* blitter_get_addr(running_machine *machine, UINT32 addr)
{
	if (addr < 0x10000)
	{
		/* Is this region fixed? */
		return (UINT8*)(memory_region(machine, "user1") + addr);
	}
	else if(addr < 0x20000)
	{
		addr &= 0xffff;
		addr += (bank_data[0] & 1) ? 0x10000 : 0;

		return (UINT8*)(memory_region(machine, "user1") + addr + ((bank_data[0] >> 1) * 0x20000));
	}
	else if (addr >= 0x20000 && addr < 0x40000)
	{
		return (UINT8*)&video_ram[addr - 0x20000];
	}
	else
	{
		return (UINT8*)&work_ram[addr - 0x40000];
	}
}


/*
    This is based this on the Slipstream technical reference manual.
    The Flare One blitter is a simpler design with slightly different parameters
    and will require hardware tests to figure everything out correctly.
*/
static void RunBlit(const address_space *space)
{
#define BLITPRG_READ(x)		blitter.x = *(blitter_get_addr(space->machine, blitter.program.addr++))

	int cycles_used = 0;


	do
	{
		UINT8 srcdata = 0;
		UINT8 dstdata = 0;

		/* Read the blitter command */
		BLITPRG_READ(source.addr0);
		BLITPRG_READ(source.addr1);
		BLITPRG_READ(source.addr2);
		BLITPRG_READ(dest.addr0);
		BLITPRG_READ(dest.addr1);
		BLITPRG_READ(dest.addr2);
		BLITPRG_READ(modectl);
		BLITPRG_READ(compfunc);
		BLITPRG_READ(outercnt);
		BLITPRG_READ(innercnt);
		BLITPRG_READ(step);
		BLITPRG_READ(pattern);

#if 0
		/* This debug is now wrong ! */
		if (DEBUG_BLITTER)
		{
			mame_printf_debug("\n%s:Blitter: Running command from 0x%.5x\n\n", cpuexec_describe_context(device->machine), blitter.program.addr - 12);
			mame_printf_debug("Command Reg         %.2x",	blitter.command);
			mame_printf_debug("		%s %s %s %s %s %s %s\n",
				blitter.command & CMD_RUN ? "RUN" : "     ",
				blitter.command & CMD_COLST ? "COLST" : "     ",
				blitter.command & CMD_PARRD ? "PARRD" : "     ",
				blitter.command & CMD_SRCUP ? "SRCUP" : "     ",
				blitter.command & CMD_DSTUP ? "DSTUP" : "     ");

			mame_printf_debug("Src Address Byte 0  %.2x\n",	blitter.source.addr0);
			mame_printf_debug("Src Address Byte 1  %.2x\n",	blitter.source.addr1);
			mame_printf_debug("Src Control         %.2x\n",	blitter.source.addr2);
			mame_printf_debug("  Src Address       %.5x\n",	blitter.source.addr & 0xfffff);
			mame_printf_debug("Dest Address Byte 0 %.2x\n",	blitter.dest.addr0);
			mame_printf_debug("Dest Address Byte 1 %.2x\n",	blitter.dest.addr1);
			mame_printf_debug("Dest Control        %.2x\n",	blitter.dest.addr2);
			mame_printf_debug("  Dst. Address      %.5x\n",	blitter.dest.addr & 0xfffff);
			mame_printf_debug("Mode Control        %.2x",	blitter.modectl);
			mame_printf_debug("		%s\n", blitter.modectl & MODE_BITTOBYTE ? "BIT_TO_BYTE" : "");

			mame_printf_debug("Comp. and LFU       %.2x\n",	blitter.compfunc);
			mame_printf_debug("Outer Loop Count    %.2x (%d)\n", blitter.outercnt, blitter.outercnt);
			mame_printf_debug("Inner Loop Count    %.2x (%d)\n", blitter.innercnt, blitter.innercnt);
			mame_printf_debug("Step Value          %.2x\n",	blitter.step);
			mame_printf_debug("Pattern Byte        %.2x\n",	blitter.pattern);
		}
#endif

		/* Ignore these writes */
		if (blitter.dest.addr == 0)
			return;

		/* Begin outer loop */
		for (;;)
		{
			UINT8 innercnt = blitter.innercnt;
			dstdata = blitter.pattern;

			if (blitter.command & CMD_LINEDRAW)
			{
				do
				{
					if (blitter.modectl & MODE_YFRAC)
					{
						if (blitter.modectl & MODE_SSIGN )
							blitter.dest.addr0--;
						else
							blitter.dest.addr0++;
					}
					else
					{
						if (blitter.modectl & MODE_DSIGN )
							blitter.dest.addr1--;
						else
							blitter.dest.addr1++;
					}
					if( blitter.source.addr0 < blitter.step )
					{

						blitter.source.addr0 -=blitter.step ;
						blitter.source.addr0 +=blitter.source.addr1;

						if ( blitter.modectl & MODE_YFRAC )
						{
							if (blitter.modectl & MODE_DSIGN )
								blitter.dest.addr1--;
							else
								blitter.dest.addr1++;
						}
						else
						{
							if (blitter.modectl & MODE_SSIGN )
								blitter.dest.addr0--;
							else
								blitter.dest.addr0++;
						}
					}
					else
					{
						blitter.source.addr0 -=blitter.step;
					}

					*blitter_get_addr(space->machine, blitter.dest.addr) = blitter.pattern;
					cycles_used++;

				} while (--innercnt);
			}
			else do
			{
				UINT8	inhibit = 0;

				/* TODO: Set this correctly */
				UINT8	result = blitter.pattern;

				if (LOOPTYPE == 3 && innercnt == blitter.innercnt)
				{
					srcdata = *(blitter_get_addr(space->machine, blitter.source.addr & 0xfffff));
					blitter.source.loword++;
					cycles_used++;
				}

				/* Enable source address read and increment? */
				if (!(blitter.modectl & (MODE_BITTOBYTE | MODE_PALREMAP)))
				{
					if (LOOPTYPE == 0 || LOOPTYPE == 1)
					{
						srcdata = *(blitter_get_addr(space->machine, blitter.source.addr & 0xfffff));
						cycles_used++;

						if (blitter.modectl & MODE_SSIGN)
							blitter.source.loword-- ;
						else
							blitter.source.loword++;

						result = srcdata;
					}
				}

				/* Read destination pixel? */
				if (LOOPTYPE == 0)
				{
					dstdata = *blitter_get_addr(space->machine, blitter.dest.addr & 0xfffff);
					cycles_used++;
				}

				/* Inhibit depending on the bit selected by the inner count */

				/* Switch on comparator type? */
				if (blitter.modectl & MODE_BITTOBYTE)
				{
					inhibit = !(srcdata & (1 << (8 - innercnt)));
				}

				if (blitter.compfunc & CMPFUNC_BEQ)
				{
					if (srcdata == blitter.pattern)
					{
						inhibit = 1;

						/* TODO: Resume from inhibit? */
						if (blitter.command & CMD_COLST)
							return;
					}
				}
				if (blitter.compfunc & CMPFUNC_LT)
				{
					/* Might be wrong */
					if ((srcdata & 0xc0) < (dstdata & 0xc0))
					{
						inhibit = 1;

						/* TODO: Resume from inhibit? */
						if (blitter.command & CMD_COLST)
							return;
					}
				}
				if (blitter.compfunc & CMPFUNC_EQ)
				{
					if ((srcdata & 0xc0) == (dstdata & 0xc0))
					{
						inhibit = 1;

						/* TODO: Resume from inhibit? */
						if (blitter.command & CMD_COLST)
							return;
					}
				}
				if (blitter.compfunc & CMPFUNC_GT)
				{
					/* Might be wrong */
					if ((srcdata & 0xc0) > (dstdata & 0xc0))
					{
						inhibit = 1;

						/* TODO: Resume from inhibit? */
						if (blitter.command & CMD_COLST)
							return;
					}
				}

				/* Write the data if not inhibited */
				if (!inhibit)
				{
					if (blitter.modectl == MODE_PALREMAP)
					{
						/*
                            In this mode, the source points to a 256 entry lookup table.
                            The existing destination pixel is used as a lookup
                            into the table and the colours is replaced.
                        */
						UINT8 dest = *blitter_get_addr(space->machine, blitter.dest.addr);
						UINT8 newcol = *(blitter_get_addr(space->machine, (blitter.source.addr + dest) & 0xfffff));

						*blitter_get_addr(space->machine, blitter.dest.addr) = newcol;
						cycles_used += 3;
					}
					else
					{
						UINT8 final_result = 0;

						if (blitter.compfunc & CMPFUNC_LOG3)
							final_result |= result & dstdata;

						if (blitter.compfunc & CMPFUNC_LOG2)
							final_result |= result & ~dstdata;

						if (blitter.compfunc & CMPFUNC_LOG1)
							final_result |= ~result & dstdata;

						if (blitter.compfunc & CMPFUNC_LOG0)
							final_result |= ~result & ~dstdata;

						*blitter_get_addr(space->machine, blitter.dest.addr) = final_result;
						cycles_used++;
					}
				}

				/* Update destination address */
				if (blitter.modectl & MODE_DSIGN)
					blitter.dest.loword--;
				else
					blitter.dest.loword++;

			} while (--innercnt);

			if (!--blitter.outercnt)
			{
				break;
			}
			else
			{
				if (blitter.command & CMD_DSTUP)
					blitter.dest.loword += blitter.step;

				if (blitter.command & CMD_SRCUP)
					blitter.source.loword += blitter.step;

				if (blitter.command & CMD_PARRD)
				{
					BLITPRG_READ(innercnt);
					BLITPRG_READ(step);
					BLITPRG_READ(pattern);
				}
			}
		}

		/* Read next command header */
		BLITPRG_READ(command);

	} while (blitter.command  & CMD_RUN);

	/* Burn Z80 cycles while blitter is in operation */
	cpu_spinuntil_time(space->cpu,  ATTOTIME_IN_NSEC( (1000000000 / Z80_XTAL)*cycles_used * 2 ) );
}


static READ8_HANDLER( ramdac_r )
{
	UINT8 val = 0xff;

	switch (offset & 3)
	{
		case 1:
		{
			UINT8 *count = &ramdac.count_r;

			if (*count == 0)
			{
				rgb_t color;
				color = palette_get_color(space->machine, ramdac.addr_r);

				ramdac.color_r[0] = RGB_RED(color);
				ramdac.color_r[1] = RGB_GREEN(color);
				ramdac.color_r[2] = RGB_BLUE(color);
			}

			val = ramdac.color_r[*count];

			/* 8bpp -> 6bpp */
			val = ((val & 0xc0) >> 2) | ((val >>2) & 0xf);

			if (++*count == 3)
			{
				*count = 0;
				ramdac.addr_r++;
			}
			break;
		}
		default:
		{
			mame_printf_debug("Unhandled RAMDAC read (PC:%.4x)\n", cpu_get_previouspc(space->cpu));
		}
	}

	return val;
}

static WRITE8_HANDLER( ramdac_w )
{

	switch (offset & 3)
	{
		case 0:
		{
			ramdac.addr_w = data;
			break;
		}
		case 1:
		{
			data &= 0x3f;
			ramdac.color_w[ramdac.count_w] = pal6bit(data);
			if (++ramdac.count_w == 3)
			{
				palette_set_color_rgb(space->machine, ramdac.addr_w, ramdac.color_w[0], ramdac.color_w[1], ramdac.color_w[2]);
				ramdac.count_w = 0;
				ramdac.addr_w++;
			}
			break;
		}
		case 2:
		{
			ramdac.mask = data;
			break;
		}
		case 3:
		{
			ramdac.addr_r = data;
			break;
		}
	}
}

/***************************************************************************

    Flare One Register Map

    01  Bank control for Z80 region 0x4000-0x7fff (16kB)    WR
    02  Bank control for Z80 region 0x8000-0xbfff (16kB)    WR
    03  Bank control for Z80 region 0xc000-0xffff (16kB)    WR

    06  Interrupt status....................................WR
    07  Interrupt ack.......................................WR
        Writing here sets the line number that vertical interrupt is generated at.
        cmd1, bit2 is the 9th bit of the line number
        ???? Written with 0x21
    08  cmd1                                                WR * bit 6 = screen select
        bit2 = 9th bit of vertical interrupt line number
        bit6 = 1 = select screen 1 else screen 0
    09  cmd2 Linked with c001...............................W * bit 0 = 1 = hires
        bit0=1=hi res else lo res (as long as bit7 is 0)
        bit5=mask msb of each pixel
        bit6=mask 2 msbits of each lores pixel
        bit7=1=variable resolution - resolution is set by bit 7 of each vram byte.  bit7=1=2 hires pixels
    0A  ???? Written with 0 and 1...........................W
        color of border

    0B  Horizontal frame buffer scroll .....................W
    0C  Vertical frame buffer scroll .......................W

    0D  Colour hold colour..................................W
    0E  Palette value for hi-res magenta....................W
    0F  Palette value for hi-res yellow?....................W
    14  ....................................................W

    18  Blitter program low byte............................WR
    19  Blitter program middle byte.........................W
    1A  Blitter program high byte...........................W
    1B  Blitter command?....................................W
    1C  Blitter status?......................................R
    20  Blitter control register............................W

    22  ???? Linked with C002...............................W
        Mask of 20

        Joystick: xxx1 11xx                                 R
        ? (polled on tight loop):  x1xx xxxx                R

    40: ROM bank select.....................................W

***************************************************************************/

static READ8_HANDLER( chipset_r )
{
	UINT8 val = 0xff;

	switch(offset)
	{
		case 1:
		case 2:
		case 3:
		{
			val = bank_data[offset];
			break;
		}
		case 6:
		{
			/* TODO */
			val = vblank_irq << 4;
			break;
		}
		case 7:
		{
			vblank_irq = 0;
			val = 0x1;

			/* TODO */
			update_irqs(space->machine);
			break;
		}
		case 0x1C:
		{
			/* Blitter status ? */
			val = 0;
			break;
		}
		case 0x20:
		{
			/* Seems correct - used during RLE pic decoding */
			val = blitter.dest.addr0;
			break;
		}
		case 0x22:
		{
			val = 0x40 | input_port_read(space->machine, "JOYSTICK");
			break;
		}
		default:
		{
			mame_printf_debug("Flare One unknown read: 0x%.2x (PC:0x%.4x)\n", offset, cpu_get_previouspc(space->cpu));
		}
	}

	return val;
}

static WRITE8_HANDLER( chipset_w )
{
	switch (offset)
	{
		case 0x01:
		case 0x02:
		case 0x03:
		{
			if (data > 0x3f)
				popmessage("%x: Unusual bank access (%x)\n", cpu_get_previouspc(space->cpu), data);

			data &= 0x3f;
			bank_data[offset] = data;
			z80_bank(space->machine, offset, data);
			break;
		}

		case 0x08:
		{
			flip_8 = data;
			break;
		}
		case 9:
			videomode = data;
			break;

		case 0x0B:
		{
			h_scroll = data;
			break;
		}
		case 0x0C:
		{
			v_scroll = data;
			break;
		}
		case 0x0E:
		{
			col4bit[5] = data;
			col3bit[5] = data;
			col3bit[5 + 8] = data;
			break;
		}
		case 0x0f:
		{
			col4bit[6] = data;
			col3bit[6] = data;
			col3bit[6 + 8] = data;
			break;
		}
		case 0x18:
		{
			blitter.program.addr0 = data;
			break;
		}
		case 0x19:
		{
			blitter.program.addr1 = data;
			break;
		}
		case 0x1A:
		{
			blitter.program.addr2 = data;
			break;
		}
		case 0x20:
		{
			blitter.command = data;

			if (data & CMD_RUN)
				RunBlit(space);
			else
				mame_printf_debug("Blitter stopped by IO.\n");

			break;
		}
		case 0x22:
		{
			flip_22 = data;
			break;
		}
		default:
		{
			mame_printf_debug("Flare One unknown write: 0x%.2x with 0x%.2x (PC:0x%.4x)\n", offset, data, cpu_get_previouspc(space->cpu));
		}
	}
}

INLINE void z80_bank(running_machine *machine, int num, int data)
{
	static const char * const bank_names[] = { "bank1", "bank2", "bank3" };

	if (data < 0x08)
	{
		UINT32 offset = ((bank_data[0] >> 1) * 0x20000) + ((0x4000 * data) ^ ((bank_data[0] & 1) ? 0 : 0x10000));

		memory_set_bankptr(machine, bank_names[num - 1], memory_region(machine, "user1") + offset);
	}
	else if (data < 0x10)
	{
		memory_set_bankptr(machine, bank_names[num - 1], &video_ram[(data - 0x08) * 0x4000]);
	}
	else
	{
		memory_set_bankptr(machine, bank_names[num - 1], &work_ram[(data - 0x10) * 0x4000]);
	}
}

static WRITE8_HANDLER( rombank_w )
{
	bank_data[0] = data;
	z80_bank(space->machine, 1, bank_data[1]);
	z80_bank(space->machine, 2, bank_data[2]);
	z80_bank(space->machine, 3, bank_data[3]);
}



/***************************************************************************

    Split into machine\cobra.c !

    Alternatively chuck it all away and borrow the MESS implementation
    because it's a million times better.

***************************************************************************/

static void command_phase(UINT8 data);
static void exec_w_phase(UINT8 data);
//UINT8 exec_r_phase(void);

/*
    WD37C656C-PL (or equivalent) Floppy Disk Controller
*/

enum fdc_phase
{
	COMMAND,
	EXECUTION_R,
	EXECUTION_W,
	RESULTS,
};

enum command
{
	SENSE_DRIVE_STATUS = 0,
	READ_A_TRACK = 2,
	SPECIFY = 3,
	WRITE_DATA = 5,
	READ_DATA = 6,
	RECALIBRATE = 7,
	SENSE_INTERRUPT_STATUS = 8,
	WRITE_DELETED_DATA = 9,
	READ_ID = 10,
	FORMAT_TRACK = 13,
	READ_DELETED_DATA = 14,
	SEEK = 15,
	SCAN_EQUAL = 17,
	SCAN_LOW_OR_EQUAL = 25,
	SCAN_HIGH_OR_EQUAL = 29
};

static struct
{
	UINT8	MSR;

	int		side;
	int		track;
	int		sector;
	int		number;
	int		stop_track;
	int		setup_read;

	int		byte_pos;
	int		offset;

	int		phase;
	int		next_phase;
	int		cmd_len;
	int		cmd_cnt;
	int		res_len;
	int		res_cnt;
	UINT8	cmd[10];
	UINT8	results[8];
} fdc;


static void reset_fdc(void)
{
	memset(&fdc, 0, sizeof(fdc));

	fdc.MSR = 0x80;
	fdc.phase = COMMAND;
}

static READ8_HANDLER( fdctrl_r )
{
	UINT8 val = 0;

	val = fdc.MSR;

	return val;
}

static READ8_HANDLER( fddata_r )
{
	#define	BPS		1024
	#define SPT		10
	#define BPT		1024*10

	UINT8 val = 0;

	if (fdc.phase == EXECUTION_R)
	{
		switch (fdc.cmd[0] & 0x1f)
		{
			/* Specify */
			case READ_DATA:
			{
				if (fdc.setup_read)
				{
					fdc.track = fdc.cmd[2];
					fdc.side = fdc.cmd[3];
					fdc.sector = fdc.cmd[4];
					fdc.number = fdc.cmd[5];
					fdc.stop_track = fdc.cmd[6];
					//int GPL = fdc.cmd[7];
					//int DTL = fdc.cmd[8];

					fdc.setup_read = 0;
					fdc.byte_pos = 0;
				}

				fdc.offset = (BPT * fdc.track*2) + (fdc.side ? BPT : 0) + (BPS * (fdc.sector-1)) + fdc.byte_pos++;
				val = *(memory_region(space->machine, "user2") + fdc.offset);

				/* Move on to next sector? */
				if (fdc.byte_pos == 1024)
				{
					fdc.byte_pos = 0;

					if (fdc.sector == fdc.stop_track || ++fdc.sector == 11)
					{
						/* End of read operation */
						fdc.MSR = 0xd0;
						fdc.phase = RESULTS;

						fdc.results[0] = 0;
						fdc.results[1] = 0;
						fdc.results[2] = 0;

						fdc.results[3] = 0;
						fdc.results[4] = 0;
						fdc.results[5] = 0;
						fdc.results[6] = 0;
					}
				}
				break;
			}
		}
	}
	else if (fdc.phase == RESULTS)
	{
		val = fdc.results[fdc.res_cnt++];

		if (fdc.res_cnt == fdc.res_len)
		{
			fdc.phase = COMMAND;
			fdc.res_cnt = 0;
			fdc.MSR &= ~0x40;
		}
	}

	return val;
}

static WRITE8_HANDLER( fdctrl_w )
{
	switch (fdc.phase)
	{
		case COMMAND:
		{
			command_phase(data);
			break;
		}
		case EXECUTION_W:
		{
			exec_w_phase(data);
			break;
		}
		default:
		{
			mame_printf_debug("Unknown FDC phase?!");
		}
	}
}

static void command_phase(UINT8 data)
{
	if (fdc.cmd_cnt == 0)
	{
		fdc.cmd[0] = data;

		fdc.cmd_cnt = 1;

		switch (data & 0x1f)
		{
			/* Specify */
			case READ_DATA:
			{
//              mame_printf_debug("Read data\n");
				fdc.cmd_len = 9;
				fdc.res_len = 7;
				fdc.next_phase = EXECUTION_R;
				fdc.setup_read = 1;
				break;
			}
			case SPECIFY:
			{
//              mame_printf_debug("Specify\n");
				fdc.cmd_len = 3;
				fdc.res_len = 0;
				fdc.next_phase = COMMAND;
				break;
			}
			case RECALIBRATE:
			{
//              mame_printf_debug("Recalibrate\n");
				fdc.cmd_len = 2;
				fdc.res_len = 0;
				fdc.next_phase = COMMAND;
				//fdc.MSR |= 0x40;
				break;
			}
			case SENSE_INTERRUPT_STATUS:
			{
//              mame_printf_debug("Sense interrupt status\n");
				fdc.cmd_len = 1;
				fdc.res_len = 2;
				fdc.phase = RESULTS;

				fdc.results[0] = 0;
				fdc.results[1] = 0;

				fdc.cmd_cnt = 0;
				fdc.MSR |= 0x40;
				break;
			}
			case SEEK:
			{
//              mame_printf_debug("Seek\n");
				fdc.cmd_len = 3;
				fdc.res_len = 0;
				fdc.next_phase = COMMAND;
				break;
			}
			default:
			{
//              mame_printf_debug("%x\n",data & 0x1f);
			}
		}
	}
	else
	{
		fdc.cmd[fdc.cmd_cnt++] = data;
		//mame_printf_debug(" %x\n",data);
	}

	if (fdc.cmd_cnt == fdc.cmd_len)
	{
		fdc.phase = fdc.next_phase;
		fdc.cmd_cnt = 0;

		if ((fdc.cmd[0] & 0x1f) == READ_DATA)
			fdc.MSR = 0xf0;
	}
}

#ifdef UNUSED_FUNCTION
UINT8 exec_r_phase(void)
{
	return 0;
}
#endif

static void exec_w_phase(UINT8 data)
{
}

#ifdef UNUSED_FUNCTION
UINT8 results_phase(void)
{
	return 0;
}

WRITE8_HANDLER( fd_op_w )
{
}

WRITE8_HANDLER( fd_ctrl_w )
{
}
#endif

static MACHINE_RESET( bfcobra )
{
	unsigned int pal;

	for (pal = 0; pal < 256; ++pal)
	{
		palette_set_color_rgb(machine, pal, pal3bit((pal>>5)&7), pal3bit((pal>>2)&7), pal2bit(pal&3));
	}

	bank_data[0] = 1;
	memset(&ramdac, 0, sizeof(ramdac));
	reset_fdc();

	irq_state = blitter_irq = vblank_irq = acia_irq = 0;
}

/***************************************************************************

    Cobra I/Viper Z80 Memory Map

***************************************************************************/

static ADDRESS_MAP_START( z80_prog_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank4")
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK("bank1")
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank2")
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank3")
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80_io_map, ADDRESS_SPACE_IO, 8 )
ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x23) AM_READWRITE(chipset_r, chipset_w)
	AM_RANGE(0x24, 0x24) AM_DEVWRITE("acia6850_0", acia6850_ctrl_w)
	AM_RANGE(0x25, 0x25) AM_DEVWRITE("acia6850_0", acia6850_data_w)
	AM_RANGE(0x26, 0x26) AM_DEVREAD("acia6850_0", acia6850_stat_r)
	AM_RANGE(0x27, 0x27) AM_DEVREAD("acia6850_0", acia6850_data_r)
	AM_RANGE(0x30, 0x30) AM_READ(fdctrl_r)
	AM_RANGE(0x31, 0x31) AM_READWRITE(fddata_r, fdctrl_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(rombank_w)
	AM_RANGE(0x50, 0x53) AM_READWRITE(ramdac_r, ramdac_w)
ADDRESS_MAP_END


/***************************************************************************

    Cobra I/Viper 6809 Memory Map

    Cobra I has these components:

    EF68B50P            - For communication with Z80
    EF68B50P            - For data retrieval
    AY38912A
    UPD7759C
    BFM1090 (CF30056)   - 'TEXAS' I/O chip, lamps and misc
    BFM1095 (CF30057)   - 'TEXAS' I/O chip, handles switches
    BFM1071             - 6809 Address decoding and bus control

    /IRQ provided by EF68850P at IC38
    /FIRQ connected to meter current sensing circuitry

    TODO: Calculate watchdog timer period.

***************************************************************************/

/* TODO */
static READ8_HANDLER( int_latch_r )
{
	return 2 | 1;
}

/* TODO */
static READ8_HANDLER( meter_r )
{
	return meter_latch;
}

/* TODO: This is borrowed from Scorpion 1 */
static WRITE8_HANDLER( meter_w )
{
	int i;
	int  changed = meter_latch ^ data;
	UINT64 cycles = downcast<cpu_device *>(space->cpu)->total_cycles();

	meter_latch = data;

	/*
        When a meter is triggered, the current drawn is sensed. If a meter
        is connected, the /FIRQ line will be pulsed.
    */
	for (i = 0; i < 8; i++)
	{
		if (changed & (1 << i))
		{
			Mechmtr_update(i, cycles, data & (1 << i) );
			generic_pulse_irq_line(space->cpu, M6809_FIRQ_LINE);
		}
	}
}

/* TODO */
static READ8_HANDLER( latch_r )
{
	return mux_input;
}

static WRITE8_HANDLER( latch_w )
{
	/* TODO: This is borrowed from Scorpion 1 */
	switch(offset)
	{
		case 0:
		{
			int changed = mux_outputlatch ^ data;
			static const char *const port[] = { "STROBE0", "STROBE1", "STROBE2", "STROBE3", "STROBE4", "STROBE5", "STROBE6", "STROBE7" };

			mux_outputlatch = data;

			/* Clock has changed */
			if (changed & 0x08)
			{
				int input_strobe = data & 0x7;

				/* Clock is low */
				if (!(data & 0x08))
					mux_input = input_port_read(space->machine, port[input_strobe]);
			}
			break;
		}
		case 1:
		{
//          strobe_data_l = data;
			break;
		}
		case 2:
		{
//          strobe_data_h = data;
			break;
		}
	}
}

static READ8_DEVICE_HANDLER( upd_r )
{
	return 2 | upd7759_busy_r(device);
}

static WRITE8_DEVICE_HANDLER( upd_w )
{
	upd7759_reset_w(device, data & 0x80);
	upd7759_port_w(device, 0, data & 0x3f);
	upd7759_start_w(device, data & 0x40 ? 0 : 1);
}

static ADDRESS_MAP_START( m6809_prog_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM	AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x2000, 0x2000) AM_RAM		// W 'B', 6F
	AM_RANGE(0x2200, 0x2200) AM_RAM		// W 'F'
	AM_RANGE(0x2600, 0x2600) AM_READWRITE(meter_r, meter_w)
	AM_RANGE(0x2800, 0x2800) AM_RAM		// W
	AM_RANGE(0x2A00, 0x2A02) AM_READWRITE(latch_r, latch_w)
	AM_RANGE(0x2E00, 0x2E00) AM_READ(int_latch_r)
	AM_RANGE(0x3001, 0x3001) AM_DEVWRITE("aysnd", ay8910_data_w)
	AM_RANGE(0x3201, 0x3201) AM_DEVWRITE("aysnd", ay8910_address_w)
	AM_RANGE(0x3404, 0x3404) AM_DEVREADWRITE("acia6850_1", acia6850_stat_r, acia6850_ctrl_w)
	AM_RANGE(0x3405, 0x3405) AM_DEVREADWRITE("acia6850_1", acia6850_data_r, acia6850_data_w)
	AM_RANGE(0x3406, 0x3406) AM_DEVREADWRITE("acia6850_2", acia6850_stat_r, acia6850_ctrl_w)
	AM_RANGE(0x3407, 0x3407) AM_DEVREADWRITE("acia6850_2", acia6850_data_r, acia6850_data_w)
//  AM_RANGE(0x3408, 0x3408) AM_NOP
//  AM_RANGE(0x340A, 0x340A) AM_NOP
//  AM_RANGE(0x3600, 0x3600) AM_NOP
	AM_RANGE(0x3801, 0x3801) AM_DEVREADWRITE("upd", upd_r, upd_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP	/* Watchdog */
ADDRESS_MAP_END

static INPUT_PORTS_START( bfcobra )
	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin: 10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("Coin: 20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("Coin: 50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("Coin: 1 pound")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Green Test?") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Red Test?")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Pass") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Continue") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Collect") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bonus") PORT_CODE(KEYCODE_F)
//  PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
//  PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
//  PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("<A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("<B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("<C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("A>")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("B>")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("C>")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_INTERLOCK) PORT_NAME("Cash box door") PORT_CODE(KEYCODE_Y) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Front Door? (resets)") PORT_CODE(KEYCODE_T) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE4")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE5")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x01, 0x00, "DIL09" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL11" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL12" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL13" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL14" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL15" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE7")
	PORT_DIPNAME( 0x01, 0x00, "DIL02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL03" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL05" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL06" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL07" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("JOYSTICK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
INPUT_PORTS_END

/*
    Allocate work RAM and video RAM shared by the Z80 and chipset.
*/
static void init_ram(running_machine *machine)
{
	/* 768kB work RAM */
	work_ram = auto_alloc_array_clear(machine, UINT8, 0xC0000);

	/* 128kB video RAM */
	video_ram = auto_alloc_array_clear(machine, UINT8, 0x20000);
}

/*
    What are the correct ACIA clocks ?
*/

static READ_LINE_DEVICE_HANDLER( z80_acia_rx_r )
{
	return m6809_z80_line;
}

static WRITE_LINE_DEVICE_HANDLER( z80_acia_tx_w )
{
	z80_m6809_line = state;
}

static WRITE_LINE_DEVICE_HANDLER( z80_acia_irq )
{
	acia_irq = state ? CLEAR_LINE : ASSERT_LINE;
	update_irqs(device->machine);
}

static ACIA6850_INTERFACE( z80_acia_if )
{
	500000,
	500000,
	DEVCB_LINE(z80_acia_rx_r), /*&m6809_z80_line,*/
	DEVCB_LINE(z80_acia_tx_w), /*&z80_m6809_line,*/
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(z80_acia_irq)
};

static READ_LINE_DEVICE_HANDLER( m6809_acia_rx_r )
{
	return z80_m6809_line;
}

static WRITE_LINE_DEVICE_HANDLER( m6809_acia_tx_w )
{
	m6809_z80_line = state;
}

static WRITE_LINE_DEVICE_HANDLER( m6809_data_irq )
{
	cputag_set_input_line(device->machine, "audiocpu", M6809_IRQ_LINE, state ? CLEAR_LINE : ASSERT_LINE);
}

static ACIA6850_INTERFACE( m6809_acia_if )
{
	500000,
	500000,
	DEVCB_LINE(m6809_acia_rx_r),/*&z80_m6809_line,*/
	DEVCB_LINE(m6809_acia_tx_w),/*&m6809_z80_line,*/
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static READ_LINE_DEVICE_HANDLER( data_acia_rx_r )
{
	return data_r;
}

static WRITE_LINE_DEVICE_HANDLER( data_acia_tx_w )
{
	 data_t = state;
}


static ACIA6850_INTERFACE( data_acia_if )
{
	500000,
	500000,
	DEVCB_LINE(data_acia_rx_r),/*data_r,*/
	DEVCB_LINE(data_acia_tx_w),/*data_t,*/
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(m6809_data_irq)
};


/* TODO: Driver vs Machine Init */
static DRIVER_INIT( bfcobra )
{
	/*
        6809 ROM address and data lines are scrambled.
        This is the same scrambling as Scorpion 2.
    */
	static const UINT8 datalookup[] = { 1, 3, 5, 6, 4, 2, 0, 7 };
	static const UINT8 addrlookup[] = { 11, 12, 0, 2, 3, 5, 7, 9, 8, 6, 1, 4, 10, 13, 14 };

	UINT32 i;
	UINT8 *rom;
	UINT8 *tmp;

	tmp = auto_alloc_array(machine, UINT8, 0x8000);
	rom = memory_region(machine, "audiocpu") + 0x8000;
	memcpy(tmp, rom, 0x8000);

	for (i = 0; i < 0x8000; i++)
	{
		UINT16 addr = 0;
		UINT8 x;
		UINT8 data = 0;
		UINT8 val = tmp[i];

		for (x = 0; x < 8; x ++)
			data |= ((val >> x) & 1) << datalookup[x];

		for (x = 0; x < 15; x ++)
			addr |= ((i >> x) & 1)  << addrlookup[x];

		rom[addr] = data;
	}

	auto_free(machine, tmp);

	init_ram(machine);

	bank_data[0] = 1;
	bank_data[1] = 0;
	bank_data[2] = 0;
	bank_data[3] = 0;

	/* Fixed 16kB ROM region */
	memory_set_bankptr(machine, "bank4", memory_region(machine, "user1"));

	/* TODO: Properly sort out the data ACIA */
	data_r = 1;

	/* Finish this */
	state_save_register_global(machine, z80_m6809_line);
	state_save_register_global(machine, m6809_z80_line);
	state_save_register_global(machine, data_r);
	state_save_register_global(machine, data_t);
	state_save_register_global(machine, h_scroll);
	state_save_register_global(machine, v_scroll);
	state_save_register_global(machine, flip_8);
	state_save_register_global(machine, flip_22);
	state_save_register_global(machine, z80_int);
	state_save_register_global(machine, z80_inten);
	state_save_register_global_array(machine, bank_data);
	state_save_register_global_pointer(machine, work_ram, 0xc0000);
	state_save_register_global_pointer(machine, video_ram, 0x20000);
}

/* TODO */
static INTERRUPT_GEN( timer_irq )
{
	generic_pulse_irq_line(device, M6809_IRQ_LINE);
}

/* TODO */
static INTERRUPT_GEN( vblank_gen )
{
	vblank_irq = 1;
	update_irqs(device->machine);
}

static MACHINE_DRIVER_START( bfcobra )
	MDRV_CPU_ADD("maincpu", Z80, Z80_XTAL)
	MDRV_CPU_PROGRAM_MAP(z80_prog_map)
	MDRV_CPU_IO_MAP(z80_io_map)
	MDRV_CPU_VBLANK_INT("screen", vblank_gen)

	MDRV_CPU_ADD("audiocpu", M6809, M6809_XTAL)
	MDRV_CPU_PROGRAM_MAP(m6809_prog_map)
	MDRV_CPU_PERIODIC_INT(timer_irq, 1000)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_MACHINE_RESET(bfcobra)

	/* TODO */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(50)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512 - 1, 0, 256 - 1)
	MDRV_PALETTE_LENGTH(256)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, M6809_XTAL)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_VIDEO_START(bfcobra)
	MDRV_VIDEO_UPDATE(bfcobra)

	/* ACIAs */
	MDRV_ACIA6850_ADD("acia6850_0", z80_acia_if)
	MDRV_ACIA6850_ADD("acia6850_1", m6809_acia_if)
	MDRV_ACIA6850_ADD("acia6850_2", data_acia_if)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

  Note: Two different versions of each 6809 ROM exist: standard and protocol
  It appears sets can be a combination of disk images, 6809 and Z80 ROMs!

***************************************************************************/

ROM_START( inquiztr )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "inq6809", 0x08000, 0x08000, CRC(ae996600) SHA1(f360399e77b81399d910770fa8106c196f04363c) )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "9576028.bin", 0x00000, 0x10000, CRC(2d85682c) SHA1(baec47bff4b8beef5afbb737dc57b22bf93ebcf8) )
	ROM_LOAD( "9576002.bin", 0x10000, 0x10000, CRC(5b8c8a04) SHA1(af5328fee79c370f45bff36f534aaf50964b6900) )

	ROM_REGION( 0x1c2000, "user2", 0 )
	ROM_LOAD( "inqdisk.img", 0x000000, 0x1c2000, NO_DUMP )
ROM_END

ROM_START( escounts )
	ROM_REGION( 0x10000, "audiocpu", 0 )
//  ROM_LOAD( "escint1b", 0x08000, 0x08000, CRC(96918aae) SHA1(849ce7b8eccc89c45aacc840a73935f95788a141) )
	ROM_LOAD( "esc12int", 0x08000, 0x08000, CRC(741a1fe6) SHA1(e741d0ae0d2f11036a358120381e4b0df4a560a1) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "esccobpa", 0x00000, 0x10000, CRC(d8eadeb7) SHA1(9b94f1454e6a17bf8321b0ef4ddd0ed1a56150f7) )

	/* 95-100-207 */
	ROM_REGION( 0x190000, "user2", 0 )
	ROM_LOAD( "escdisk4.img", 0x000000, 0x190000, CRC(24156e0f) SHA1(e163daa8effadd93ace2bc2ba1af0f4a190abd7f) )
ROM_END

ROM_START( trebltop )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "95740078.bin", 0x08000, 0x08000, CRC(aca1980b) SHA1(3d4ed1dc545cc80f56d7daa13028fb10a12a718b) )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "95760031.bin", 0x00000, 0x10000, CRC(8e75edb8) SHA1(0aaa3834d5ac20f92bdf1f2b8f1eb71854469cbe) )
	ROM_LOAD( "95760021.bin", 0x10000, 0x10000, CRC(f42016c0) SHA1(7067d018cb4bdcfba777267fb01cddf44e4216c3) )

	ROM_REGION( 0x1c2000, "user2", 0 )
	ROM_LOAD( "ttdisk1.img", 0x000000, 0x190000, CRC(b2003228) SHA1(5eb49f05137cdd404f22948d39aa79c1518c06eb) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "95000172.bin", 0x00000, 0x10000, CRC(e85367a5) SHA1(695fd95ddeecdb16602f7b0f075cf5128a2fb808) )
	ROM_LOAD( "95000173.bin", 0x10000, 0x10000, CRC(8bda2c5e) SHA1(79aab5a2af7a5add5fe9132dc13bcc3705c6faf3) )
ROM_END

ROM_START( beeline )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bln12int.a", 0x08000, 0x08000, CRC(cb97905e) SHA1(9725156bf64e53a56bc0f90795d4b07db41d059e) )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "blncob.pa", 0x00000, 0x10000, CRC(8abc0017) SHA1(ecf6e7a4021b35295eb9bb9aed1b88fff27ffbd1) )
	ROM_LOAD( "blncob.pb", 0x10000, 0x10000, CRC(feb121fe) SHA1(e83bfd6db00a3264e5076f257e261a1cb4605a83) )

	ROM_REGION( 0x1c2000, "user2", 0 )
	ROM_LOAD( "beedisk.img", 0x000000, 0x1c2000, NO_DUMP )
ROM_END

ROM_START( quizvadr )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "q6809.bin", 0x08000, 0x8000, CRC(a74dff10) SHA1(87578694a022dc3d7ade9cc76d387c1ae5fc74d9) )

	ROM_REGION( 0x200000, "user1", 0 )
	ROM_LOAD( "5947011r.0", 0x000000, 0x80000, CRC(cac43c97) SHA1(3af529cd0f8ec57dd3596f5bca7b9c74cff171e4) )
	ROM_LOAD( "5947011r.1", 0x080000, 0x80000, CRC(120018dc) SHA1(cd153d2b7ed535b04dbcaf189d2fc96fe3c5b466) )
	ROM_LOAD( "5947011r.2", 0x100000, 0x80000, CRC(689b3b5a) SHA1(ea32d18acfd380de822efff4f2c95ce9873a33a2) )
	ROM_LOAD( "5947011r.3", 0x180000, 0x80000, CRC(c38dafeb) SHA1(d693387a5c3cde34c9d581f81a08a5fbc6f753f2) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "185snd2.bin", 0x00000, 0x10000, CRC(e36eccc2) SHA1(cfd8ca4c71528ea4e229074016240681b6de37cd) )
	ROM_LOAD( "184snd1.bin", 0x10000, 0x10000, CRC(aac058e8) SHA1(39e59ad9524130fc3bd8d46e1aa78bc4daf04e39) )
ROM_END

ROM_START( qos )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "39360107.bin", 0x08000, 0x8000, CRC(20844655) SHA1(b67c7f7bbabf6d5139b8ad8cbb5f8cc3f28e9cc7) )

	ROM_REGION( 0x200000, "user1", 0 )
	ROM_LOAD( "95000338.rm0", 0x000000, 0x80000, CRC(96918aae) SHA1(849ce7b8eccc89c45aacc840a73935f95788a141) )
	ROM_LOAD( "95000339.rm1", 0x080000, 0x80000, CRC(b4c6dcc0) SHA1(56d8761766dfbd5b0e71f8c3ca575e88f1bc9929) )
	ROM_LOAD( "95000340.rm2", 0x100000, 0x80000, CRC(66d121fd) SHA1(ac65cc0ac6b0a41e78a3159c21ee44f765bdb5c8) )
	ROM_LOAD( "95000341.rm3", 0x180000, 0x80000, CRC(ef13658d) SHA1(240bc589900214eac79c91a531f254a9ac2f4ef6) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "snd1_218.ic7", 0x00000, 0x10000, CRC(061f496d) SHA1(653d16454d909c034191813b37d14010da7258c6) )
	ROM_LOAD( "snd2_219.ic8", 0x10000, 0x10000, CRC(d7874a47) SHA1(5bbd4040c7c0299e8cc135e6c6cd05370b260e9b) )
ROM_END

ROM_START( qosa )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "qos_nondata_68f4.bin", 0x08000, 0x8000, CRC(5f40005a) SHA1(180017acf6b432bc135d1090099fdf99f1e3583a) )

	ROM_REGION( 0x200000, "user1", 0 )
	ROM_LOAD( "95000338.rm0", 0x000000, 0x80000, CRC(96918aae) SHA1(849ce7b8eccc89c45aacc840a73935f95788a141) )
	ROM_LOAD( "95000339.rm1", 0x080000, 0x80000, CRC(b4c6dcc0) SHA1(56d8761766dfbd5b0e71f8c3ca575e88f1bc9929) )
	ROM_LOAD( "95000340.rm2", 0x100000, 0x80000, CRC(66d121fd) SHA1(ac65cc0ac6b0a41e78a3159c21ee44f765bdb5c8) )
	ROM_LOAD( "95000341.rm3", 0x180000, 0x80000, CRC(ef13658d) SHA1(240bc589900214eac79c91a531f254a9ac2f4ef6) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "snd1_218.ic7", 0x00000, 0x10000, CRC(061f496d) SHA1(653d16454d909c034191813b37d14010da7258c6) )
	ROM_LOAD( "snd2_219.ic8", 0x10000, 0x10000, CRC(d7874a47) SHA1(5bbd4040c7c0299e8cc135e6c6cd05370b260e9b) )
ROM_END

ROM_START( qosb )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "95740599.bin", 0x08000, 0x8000, CRC(bf1e321f) SHA1(51f18620f22ba2a1b110954284ddf00614d51a0e) )

	ROM_REGION( 0x200000, "user1", 0 )
	ROM_LOAD( "0306.bin", 0x000000, 0x80000, CRC(c26c8f83) SHA1(6949027e1fe241cbb2e1cbbce18e47bcb0d84550) )
	ROM_LOAD( "1307.bin", 0x080000, 0x80000, CRC(94611c03) SHA1(81f545ff96ff3d44285315400da94d870c89f896) )
	ROM_LOAD( "2308.bin", 0x100000, 0x80000, CRC(f5572726) SHA1(e109265c5571d21213a6f405a13459e7bc6699bc) )
	ROM_LOAD( "3309.bin", 0x180000, 0x80000, CRC(1b5edfa8) SHA1(348488debd4aa52f064e351ed0c082274da1db2b) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "snd1_218.ic7", 0x00000, 0x10000, CRC(061f496d) SHA1(653d16454d909c034191813b37d14010da7258c6) )
	ROM_LOAD( "snd2_219.ic8", 0x10000, 0x10000, CRC(d7874a47) SHA1(5bbd4040c7c0299e8cc135e6c6cd05370b260e9b) )
ROM_END

GAME( 1989, inquiztr, 0,   bfcobra, bfcobra, bfcobra, ROT0, "BFM", "Inquizitor",                       GAME_NOT_WORKING )
GAME( 1990, escounts, 0,   bfcobra, bfcobra, bfcobra, ROT0, "BFM", "Every Second Counts (39-360-053)", GAME_IMPERFECT_GRAPHICS )
GAME( 1991, trebltop, 0,   bfcobra, bfcobra, bfcobra, ROT0, "BFM", "Treble Top (39-360-070)",          GAME_IMPERFECT_GRAPHICS )
GAME( 1991, beeline,  0,   bfcobra, bfcobra, bfcobra, ROT0, "BFM", "Beeline (39-360-075)",             GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )
GAME( 1991, quizvadr, 0,   bfcobra, bfcobra, bfcobra, ROT0, "BFM", "Quizvaders (39-360-078)",          GAME_IMPERFECT_GRAPHICS )
GAME( 1992, qos,      0,   bfcobra, bfcobra, bfcobra, ROT0, "BFM", "A Question of Sport (39-960-107)", GAME_IMPERFECT_GRAPHICS )
GAME( 1992, qosa,     qos, bfcobra, bfcobra, bfcobra, ROT0, "BFM", "A Question of Sport (39-960-099)", GAME_IMPERFECT_GRAPHICS )
GAME( 1992, qosb,     qos, bfcobra, bfcobra, bfcobra, ROT0, "BFM", "A Question of Sport (39-960-089)", GAME_IMPERFECT_GRAPHICS )
