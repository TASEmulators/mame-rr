/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/

#include "emu.h"
#include "cpu/tms32031/tms32031.h"
#include "includes/midzeus.h"
#include "video/poly.h"
#include "video/rgbutil.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define DUMP_WAVE_RAM		0
#define TRACK_REG_USAGE		0

#define WAVERAM0_WIDTH		1024
#define WAVERAM0_HEIGHT		2048

#define WAVERAM1_WIDTH		512
#define WAVERAM1_HEIGHT		1024



/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	const void *	palbase;
	const void *	texbase;
	UINT16			solidcolor;
	INT16			zoffset;
	UINT16			transcolor;
	UINT16			texwidth;
	UINT16			color;
	UINT32			alpha;
};



/*************************************
 *
 *  Global variables
 *
 *************************************/

static poly_manager *poly;
static UINT8 log_fifo;

static UINT32 zeus_fifo[20];
static UINT8 zeus_fifo_words;
static void *zeus_renderbase;
static rectangle zeus_cliprect;

static float zeus_matrix[3][3];
static float zeus_point[3];
static float zeus_point2[3];
static UINT32 zeus_texbase;
static UINT32 zeus_unknown_40;
static int zeus_quad_size;

static UINT32 *waveram[2];
static emu_timer *int_timer;
static int yoffs;
static int texel_width;
static float zbase;

#if TRACK_REG_USAGE
typedef struct reg_info
{
	struct reg_info *next;
	UINT32 value;
} reg_info;

static reg_info *regdata[0x80];
static int regdata_count[0x80];
static int regread_count[0x80];
static int regwrite_count[0x80];
static reg_info *subregdata[0x100];
static int subregdata_count[0x80];
static int subregwrite_count[0x100];

#endif



/*************************************
 *
 *  Function prototypes
 *
 *************************************/

static void exit_handler(running_machine &machine);

static void zeus_register32_w(running_machine *machine, offs_t offset, UINT32 data, int logit);
static void zeus_register_update(running_machine *machine, offs_t offset, UINT32 oldval, int logit);
static void zeus_pointer_write(UINT8 which, UINT32 value);
static int zeus_fifo_process(running_machine *machine, const UINT32 *data, int numwords);
static void zeus_draw_model(running_machine *machine, UINT32 baseaddr, UINT16 count, int logit);
static void zeus_draw_quad(running_machine *machine, const UINT32 *databuffer, UINT32 texoffs, int logit);
static void render_poly_8bit(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);

static void log_fifo_command(const UINT32 *data, int numwords, const char *suffix);
//static void log_waveram(UINT32 base, UINT16 length);



/*************************************
 *
 *  Macros
 *
 *************************************/

#define WAVERAM_BLOCK0(blocknum)				((void *)((UINT8 *)waveram[0] + 8 * (blocknum)))
#define WAVERAM_BLOCK1(blocknum)				((void *)((UINT8 *)waveram[1] + 12 * (blocknum)))

#define WAVERAM_PTR8(base, bytenum)				((UINT8 *)(base) + BYTE4_XOR_LE(bytenum))
#define WAVERAM_READ8(base, bytenum)			(*WAVERAM_PTR8(base, bytenum))
#define WAVERAM_WRITE8(base, bytenum, data)		do { *WAVERAM_PTR8(base, bytenum) = (data); } while (0)

#define WAVERAM_PTR16(base, wordnum)			((UINT16 *)(base) + BYTE_XOR_LE(wordnum))
#define WAVERAM_READ16(base, wordnum)			(*WAVERAM_PTR16(base, wordnum))
#define WAVERAM_WRITE16(base, wordnum, data)	do { *WAVERAM_PTR16(base, wordnum) = (data); } while (0)

#define WAVERAM_PTR32(base, dwordnum)			((UINT32 *)(base) + (dwordnum))
#define WAVERAM_READ32(base, dwordnum)			(*WAVERAM_PTR32(base, dwordnum))
#define WAVERAM_WRITE32(base, dwordnum, data)	do { *WAVERAM_PTR32(base, dwordnum) = (data); } while (0)

#define PIXYX_TO_DWORDNUM(y, x)					(((((y) & 0x1ff) << 8) | (((x) & 0x1fe) >> 1)) * 3 + ((x) & 1))
#define DEPTHYX_TO_DWORDNUM(y, x)				(PIXYX_TO_DWORDNUM(y, (x) & ~1) + 2)

#define WAVERAM_PTRPIX(base, y, x)				WAVERAM_PTR32(base, PIXYX_TO_DWORDNUM(y, x))
#define WAVERAM_READPIX(base, y, x)				(*WAVERAM_PTRPIX(base, y, x))
#define WAVERAM_WRITEPIX(base, y, x, color)		do { *WAVERAM_PTRPIX(base, y, x) = (color);  } while (0)

#define WAVERAM_PTRDEPTH(base, y, x)			WAVERAM_PTR16(base, DEPTHYX_TO_DWORDNUM(y, x) * 2 + (x & 1))
#define WAVERAM_READDEPTH(base, y, x)			(*WAVERAM_PTRDEPTH(base, y, x))
#define WAVERAM_WRITEDEPTH(base, y, x, color)	do { *WAVERAM_PTRDEPTH(base, y, x) = (color);  } while (0)



/*************************************
 *
 *  Inlines for block addressing
 *
 *************************************/

INLINE void *waveram0_ptr_from_expanded_addr(UINT32 addr)
{
	UINT32 blocknum = (addr % WAVERAM0_WIDTH) + ((addr >> 16) % WAVERAM0_HEIGHT) * WAVERAM0_WIDTH;
	return WAVERAM_BLOCK0(blocknum);
}

INLINE void *waveram1_ptr_from_expanded_addr(UINT32 addr)
{
	UINT32 blocknum = (addr % WAVERAM1_WIDTH) + ((addr >> 16) % WAVERAM1_HEIGHT) * WAVERAM1_WIDTH;
	return WAVERAM_BLOCK1(blocknum);
}

#ifdef UNUSED_FUNCTION
INLINE void *waveram0_ptr_from_texture_addr(UINT32 addr, int width)
{
	UINT32 blocknum = ((addr & ~1) * width) / 8;
	return WAVERAM_BLOCK0(blocknum);
}
#endif


/*************************************
 *
 *  Inlines for rendering
 *
 *************************************/

#ifdef UNUSED_FUNCTION
INLINE void waveram_plot(int y, int x, UINT32 color)
{
	if (x >= 0 && x <= zeus_cliprect.max_x && y >= 0 && y < zeus_cliprect.max_y)
		WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
}
#endif

INLINE void waveram_plot_depth(int y, int x, UINT32 color, UINT16 depth)
{
	if (x >= 0 && x <= zeus_cliprect.max_x && y >= 0 && y < zeus_cliprect.max_y)
	{
		WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
		WAVERAM_WRITEDEPTH(zeus_renderbase, y, x, depth);
	}
}

#ifdef UNUSED_FUNCTION
INLINE void waveram_plot_check_depth(int y, int x, UINT32 color, UINT16 depth)
{
	if (x >= 0 && x <= zeus_cliprect.max_x && y >= 0 && y < zeus_cliprect.max_y)
	{
		UINT16 *depthptr = WAVERAM_PTRDEPTH(zeus_renderbase, y, x);
		if (depth <= *depthptr)
		{
			WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
			*depthptr = depth;
		}
	}
}
#endif

#ifdef UNUSED_FUNCTION
INLINE void waveram_plot_check_depth_nowrite(int y, int x, UINT32 color, UINT16 depth)
{
	if (x >= 0 && x <= zeus_cliprect.max_x && y >= 0 && y < zeus_cliprect.max_y)
	{
		UINT16 *depthptr = WAVERAM_PTRDEPTH(zeus_renderbase, y, x);
		if (depth <= *depthptr)
			WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
	}
}
#endif


/*************************************
 *
 *  Inlines for texel accesses
 *
 *************************************/

INLINE UINT8 get_texel_8bit(const void *base, int y, int x, int width)
{
	UINT32 byteoffs = (y / 2) * (width * 2) + ((x / 4) << 3) + ((y & 1) << 2) + (x & 3);
	return WAVERAM_READ8(base, byteoffs);
}


#ifdef UNUSED_FUNCTION
INLINE UINT8 get_texel_4bit(const void *base, int y, int x, int width)
{
	UINT32 byteoffs = (y / 2) * (width * 2) + ((x / 8) << 3) + ((y & 1) << 2) + ((x / 2) & 3);
	return (WAVERAM_READ8(base, byteoffs) >> (4 * (x & 1))) & 0x0f;
}
#endif


/*************************************
 *
 *  Video startup
 *
 *************************************/

static TIMER_CALLBACK( int_timer_callback )
{
	cputag_set_input_line(machine, "maincpu", 2, ASSERT_LINE);
}


VIDEO_START( midzeus2 )
{
	/* allocate memory for "wave" RAM */
	waveram[0] = auto_alloc_array(machine, UINT32, WAVERAM0_WIDTH * WAVERAM0_HEIGHT * 8/4);
	waveram[1] = auto_alloc_array(machine, UINT32, WAVERAM1_WIDTH * WAVERAM1_HEIGHT * 12/4);

	/* initialize polygon engine */
	poly = poly_alloc(machine, 10000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);

	/* we need to cleanup on exit */
	machine->add_notifier(MACHINE_NOTIFY_EXIT, exit_handler);

	zbase = 2.0f;
	yoffs = 0;
	texel_width = 256;
	zeus_renderbase = waveram[1];

	int_timer = timer_alloc(machine, int_timer_callback, NULL);

	/* save states */
	state_save_register_global_pointer(machine, waveram[0], WAVERAM0_WIDTH * WAVERAM0_HEIGHT * 8 / sizeof(waveram[0][0]));
	state_save_register_global_pointer(machine, waveram[1], WAVERAM1_WIDTH * WAVERAM1_HEIGHT * 12 / sizeof(waveram[1][0]));
	state_save_register_global_array(machine, zeus_fifo);
	state_save_register_global(machine, zeus_fifo_words);
	state_save_register_global(machine, zeus_cliprect.min_x);
	state_save_register_global(machine, zeus_cliprect.max_x);
	state_save_register_global(machine, zeus_cliprect.min_y);
	state_save_register_global(machine, zeus_cliprect.max_y);
	state_save_register_global_2d_array(machine, zeus_matrix);
	state_save_register_global_array(machine, zeus_point);
	state_save_register_global(machine, zeus_texbase);
}


static void exit_handler(running_machine &machine)
{
#if DUMP_WAVE_RAM
	FILE *f = fopen("waveram.dmp", "w");
	int i;

	for (i = 0; i < WAVERAM0_WIDTH * WAVERAM0_HEIGHT; i++)
	{
		if (i % 4 == 0) fprintf(f, "%03X%03X: ", i / WAVERAM0_WIDTH, i % WAVERAM0_WIDTH);
		fprintf(f, " %08X %08X ",
			WAVERAM_READ32(waveram[0], i*2+0),
			WAVERAM_READ32(waveram[0], i*2+1));
		if (i % 4 == 3) fprintf(f, "\n");
	}
	fclose(f);
#endif

#if TRACK_REG_USAGE
{
	reg_info *info;
	int regnum;

	for (regnum = 0; regnum < 0x80; regnum++)
	{
		printf("Register %02X\n", regnum);
		if (regread_count[regnum] == 0)
			printf("\tNever read\n");
		else
			printf("\tRead %d times\n", regread_count[regnum]);

		if (regwrite_count[regnum] == 0)
			printf("\tNever written\n");
		else
		{
			printf("\tWritten %d times\n", regwrite_count[regnum]);
			for (info = regdata[regnum]; info != NULL; info = info->next)
				printf("\t%08X\n", info->value);
		}
	}

	for (regnum = 0; regnum < 0x100; regnum++)
		if (subregwrite_count[regnum] != 0)
		{
			printf("Sub-Register %02X (%d writes)\n", regnum, subregwrite_count[regnum]);
			for (info = subregdata[regnum]; info != NULL; info = info->next)
				printf("\t%08X\n", info->value);
		}
}
#endif

	poly_free(poly);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( midzeus2 )
{
	int x, y;

	poly_wait(poly, "VIDEO_UPDATE");

if (input_code_pressed(screen->machine, KEYCODE_UP)) { zbase += 1.0f; popmessage("Zbase = %f", zbase); }
if (input_code_pressed(screen->machine, KEYCODE_DOWN)) { zbase -= 1.0f; popmessage("Zbase = %f", zbase); }

	/* normal update case */
	if (!input_code_pressed(screen->machine, KEYCODE_W))
	{
		const void *base = waveram1_ptr_from_expanded_addr(zeusbase[0x38]);
		int xoffs = screen->visible_area().min_x;
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT32 *dest = (UINT32 *)bitmap->base + y * bitmap->rowpixels;
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
				dest[x] = WAVERAM_READPIX(base, y, x - xoffs);
		}
	}

	/* waveram drawing case */
	else
	{
		const UINT64 *base;

		if (input_code_pressed(screen->machine, KEYCODE_DOWN)) yoffs += input_code_pressed(screen->machine, KEYCODE_LSHIFT) ? 0x40 : 1;
		if (input_code_pressed(screen->machine, KEYCODE_UP)) yoffs -= input_code_pressed(screen->machine, KEYCODE_LSHIFT) ? 0x40 : 1;
		if (input_code_pressed(screen->machine, KEYCODE_LEFT) && texel_width > 4) { texel_width >>= 1; while (input_code_pressed(screen->machine, KEYCODE_LEFT)) ; }
		if (input_code_pressed(screen->machine, KEYCODE_RIGHT) && texel_width < 512) { texel_width <<= 1; while (input_code_pressed(screen->machine, KEYCODE_RIGHT)) ; }

		if (yoffs < 0) yoffs = 0;
		base = (const UINT64 *)waveram0_ptr_from_expanded_addr(yoffs << 16);

		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT32 *dest = (UINT32 *)bitmap->base + y * bitmap->rowpixels;
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				UINT8 tex = get_texel_8bit(base, y, x, texel_width);
				dest[x] = (tex << 16) | (tex << 8) | tex;
			}
		}
		popmessage("offs = %06X", yoffs << 12);
	}

	return 0;
}



/*************************************
 *
 *  Core read handler
 *
 *************************************/

READ32_HANDLER( zeus2_r )
{
	int logit = (offset != 0x00 && offset != 0x01 && offset != 0x54 && offset != 0x48 && offset != 0x49 && offset != 0x58 && offset != 0x59 && offset != 0x5a);
	UINT32 result = zeusbase[offset];

#if TRACK_REG_USAGE
	regread_count[offset]++;
#endif

	if (logit)
		logerror("%06X:zeus2_r(%02X)\n", cpu_get_pc(space->cpu), offset);

	switch (offset)
	{
		case 0x00:
			result = 0x20;
			break;

		case 0x01:
			/* bit  $000C0070 are tested in a loop until 0 */
			/* bits $00080000 is tested in a loop until 0 */
			/* bit  $00000004 is tested for toggling; probably VBLANK */
			result = 0x00;
			if (space->machine->primary_screen->vblank())
				result |= 0x04;
			break;

		case 0x07:
			/* this is needed to pass the self-test in thegrid */
			result = 0x10451998;
			break;

		case 0x54:
			/* both upper 16 bits and lower 16 bits seem to be used as vertical counters */
			result = (space->machine->primary_screen->vpos() << 16) | space->machine->primary_screen->vpos();
			break;
	}

	return result;
}



/*************************************
 *
 *  Core write handler
 *
 *************************************/

WRITE32_HANDLER( zeus2_w )
{
	int logit = (offset != 0x08 &&
				 (offset != 0x20 || data != 0) &&
				 offset != 0x40 && offset != 0x41 && offset != 0x48 && offset != 0x49 && offset != 0x4e &&
				 offset != 0x50 && offset != 0x51 && offset != 0x57 && offset != 0x58 && offset != 0x59 && offset != 0x5a && offset != 0x5e);
	if (logit)
		logerror("%06X:zeus2_w", cpu_get_pc(space->cpu));
	zeus_register32_w(space->machine, offset, data, logit);
}



/*************************************
 *
 *  Handle register writes
 *
 *************************************/

static void zeus_register32_w(running_machine *machine, offs_t offset, UINT32 data, int logit)
{
	UINT32 oldval = zeusbase[offset];

#if TRACK_REG_USAGE
regwrite_count[offset]++;
if (regdata_count[offset] < 256)
{
	reg_info **tailptr;

	for (tailptr = &regdata[offset]; *tailptr != NULL; tailptr = &(*tailptr)->next)
		if ((*tailptr)->value == data)
			break;
	if (*tailptr == NULL)
	{
		*tailptr = alloc_or_die(reg_info);
		(*tailptr)->next = NULL;
		(*tailptr)->value = data;
		regdata_count[offset]++;
	}
}
#endif

	/* writes to register $CC need to force a partial update */
//  if ((offset & ~1) == 0xcc)
//      machine->primary_screen->update_partial(machine->primary_screen->vpos());

	/* always write to low word? */
	zeusbase[offset] = data;

	/* log appropriately */
	if (logit)
		logerror("(%02X) = %08X\n", offset, data);

	/* handle the update */
	zeus_register_update(machine, offset, oldval, logit);
}



/*************************************
 *
 *  Update state after a register write
 *
 *************************************/

static void zeus_register_update(running_machine *machine, offs_t offset, UINT32 oldval, int logit)
{
	/* handle the writes; only trigger on low accesses */
	switch (offset)
	{
		case 0x08:
			zeus_fifo[zeus_fifo_words++] = zeusbase[0x08];
			if (zeus_fifo_process(machine, zeus_fifo, zeus_fifo_words))
				zeus_fifo_words = 0;

			/* set the interrupt signal to indicate we can handle more */
			timer_adjust_oneshot(int_timer, ATTOTIME_IN_NSEC(500), 0);
			break;

		case 0x20:
			/* toggles between two values based on the page:

                Page #      zeusbase[0x20]      zeusbase[0x38]
                ------      --------------      --------------
                   0          $04000190           $00000000
                   1          $04000000           $01900000
            */
            zeus_pointer_write(zeusbase[0x20] >> 24, zeusbase[0x20]);
			break;

		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
			machine->primary_screen->update_partial(machine->primary_screen->vpos());
			{
				int vtotal = zeusbase[0x37] & 0xffff;
				int htotal = zeusbase[0x34] >> 16;
				rectangle visarea;

				visarea.min_x = zeusbase[0x33] >> 16;
				visarea.max_x = (zeusbase[0x34] & 0xffff) - 1;
				visarea.min_y = 0;
				visarea.max_y = zeusbase[0x35] & 0xffff;
				if (htotal > 0 && vtotal > 0 && visarea.min_x < visarea.max_x && visarea.max_y < vtotal)
				{
					machine->primary_screen->configure(htotal, vtotal, visarea, HZ_TO_ATTOSECONDS((double)MIDZEUS_VIDEO_CLOCK / 4.0 / (htotal * vtotal)));
					zeus_cliprect = visarea;
					zeus_cliprect.max_x -= zeus_cliprect.min_x;
					zeus_cliprect.min_x = 0;
				}
			}
			break;

		case 0x38:
			{
				UINT32 temp = zeusbase[0x38];
				zeusbase[0x38] = oldval;
				machine->primary_screen->update_partial(machine->primary_screen->vpos());
				log_fifo = input_code_pressed(machine, KEYCODE_L);
				zeusbase[0x38] = temp;
			}
			break;

		case 0x41:
			/* this is the address, except in read mode, where it latches values */
			if (zeusbase[0x4e] & 0x10)
			{
				const void *src = waveram0_ptr_from_expanded_addr(oldval);
				zeusbase[0x41] = oldval;
				zeusbase[0x48] = WAVERAM_READ32(src, 0);
				zeusbase[0x49] = WAVERAM_READ32(src, 1);

				if (zeusbase[0x4e] & 0x40)
				{
					zeusbase[0x41]++;
					zeusbase[0x41] += (zeusbase[0x41] & 0x400) << 6;
					zeusbase[0x41] &= ~0xfc00;
				}
			}
			break;

		case 0x48:
		case 0x49:
			/* if we're in write mode, process it */
			if (zeusbase[0x40] == 0x00890000)
			{
				/*
                    zeusbase[0x4e]:
                        bit 0-1: which register triggers write through
                        bit 3:   enable write through via these registers
                        bit 4:   seems to be set during reads, when 0x41 is used for latching
                        bit 6:   enable autoincrement on write through
                */
				if ((zeusbase[0x4e] & 0x08) && (offset & 3) == (zeusbase[0x4e] & 3))
				{
					void *dest = waveram0_ptr_from_expanded_addr(zeusbase[0x41]);
					WAVERAM_WRITE32(dest, 0, zeusbase[0x48]);
					WAVERAM_WRITE32(dest, 1, zeusbase[0x49]);

					if (zeusbase[0x4e] & 0x40)
					{
						zeusbase[0x41]++;
						zeusbase[0x41] += (zeusbase[0x41] & 0x400) << 6;
						zeusbase[0x41] &= ~0xfc00;
					}
				}
			}

			/* make sure we log anything else */
			else if (logit)
				logerror("\t[40]=%08X [4E]=%08X\n", zeusbase[0x40], zeusbase[0x4e]);
			break;

		case 0x51:

			/* in this mode, crusnexo expects the reads to immediately latch */
			if (zeusbase[0x50] == 0x00a20000)
				oldval = zeusbase[0x51];

			/* this is the address, except in read mode, where it latches values */
			if ((zeusbase[0x5e] & 0x10) || (zeusbase[0x50] == 0x00a20000))
			{
				const void *src = waveram1_ptr_from_expanded_addr(oldval);
				zeusbase[0x51] = oldval;
				zeusbase[0x58] = WAVERAM_READ32(src, 0);
				zeusbase[0x59] = WAVERAM_READ32(src, 1);
				zeusbase[0x5a] = WAVERAM_READ32(src, 2);

				if (zeusbase[0x5e] & 0x40)
				{
					zeusbase[0x51]++;
					zeusbase[0x51] += (zeusbase[0x51] & 0x200) << 7;
					zeusbase[0x51] &= ~0xfe00;
				}
			}
			break;

		case 0x57:
			/* thegrid uses this to write either left or right halves of pixels */
			if (zeusbase[0x50] == 0x00e90000)
			{
				void *dest = waveram1_ptr_from_expanded_addr(zeusbase[0x51]);
				if (zeusbase[0x57] & 1)
					WAVERAM_WRITE32(dest, 0, zeusbase[0x58]);
				if (zeusbase[0x57] & 4)
					WAVERAM_WRITE32(dest, 1, zeusbase[0x59]);
			}

			/* make sure we log anything else */
			else if (logit)
				logerror("\t[50]=%08X [5E]=%08X\n", zeusbase[0x50], zeusbase[0x5e]);
			break;

		case 0x58:
		case 0x59:
		case 0x5a:
			/* if we're in write mode, process it */
			if (zeusbase[0x50] == 0x00890000)
			{
				/*
                    zeusbase[0x5e]:
                        bit 0-1: which register triggers write through
                        bit 3:   enable write through via these registers
                        bit 4:   seems to be set during reads, when 0x51 is used for latching
                        bit 5:   unknown, currently used to specify ordering, but this is suspect
                        bit 6:   enable autoincrement on write through
                */
				if ((zeusbase[0x5e] & 0x08) && (offset & 3) == (zeusbase[0x5e] & 3))
				{
					void *dest = waveram1_ptr_from_expanded_addr(zeusbase[0x51]);
					WAVERAM_WRITE32(dest, 0, zeusbase[0x58]);
					if (zeusbase[0x5e] & 0x20)
						WAVERAM_WRITE32(dest, 1, zeusbase[0x5a]);
					else
					{
						WAVERAM_WRITE32(dest, 1, zeusbase[0x59]);
						WAVERAM_WRITE32(dest, 2, zeusbase[0x5a]);
					}

					if (zeusbase[0x5e] & 0x40)
					{
						zeusbase[0x51]++;
						zeusbase[0x51] += (zeusbase[0x51] & 0x200) << 7;
						zeusbase[0x51] &= ~0xfe00;
					}
				}
			}

			/* make sure we log anything else */
			else if (logit)
				logerror("\t[50]=%08X [5E]=%08X\n", zeusbase[0x50], zeusbase[0x5e]);
			break;
	}
}



/*************************************
 *
 *  Process the FIFO
 *
 *************************************/

static void zeus_pointer_write(UINT8 which, UINT32 value)
{
#if TRACK_REG_USAGE
subregwrite_count[which]++;
if (subregdata_count[which] < 256)
{
	reg_info **tailptr;

	for (tailptr = &subregdata[which]; *tailptr != NULL; tailptr = &(*tailptr)->next)
		if ((*tailptr)->value == value)
			break;
	if (*tailptr == NULL)
	{
		*tailptr = alloc_or_die(reg_info);
		(*tailptr)->next = NULL;
		(*tailptr)->value = value;
		subregdata_count[which]++;
	}
}
#endif

	switch (which)
	{
		case 0x04:
			zeus_renderbase = waveram1_ptr_from_expanded_addr(value << 16);
			break;

		case 0x05:
			zeus_texbase = value % (WAVERAM0_HEIGHT * WAVERAM0_WIDTH);
			break;

		case 0x40:
			zeus_unknown_40 = value & 0xffffff;
			zeus_quad_size = (zeus_unknown_40 == 0) ? 10 : 14;
			break;
	}
}



/*************************************
 *
 *  Process the FIFO
 *
 *************************************/

static int zeus_fifo_process(running_machine *machine, const UINT32 *data, int numwords)
{
	int dataoffs = 0;

	/* handle logging */
	switch (data[0] >> 24)
	{
		/* 0x05: write 32-bit value to low registers */
		case 0x05:
			if (numwords < 2)
				return FALSE;
			if (log_fifo)
				log_fifo_command(data, numwords, " -- reg32");
			if (((data[0] >> 16) & 0x7f) != 0x08)
				zeus_register32_w(machine, (data[0] >> 16) & 0x7f, data[1], log_fifo);
			break;

		/* 0x08: set matrix and point (thegrid) */
		case 0x08:
			if (numwords < 14)
				return FALSE;
			dataoffs = 1;

		/* 0x07: set matrix and point (crusnexo) */
		case 0x07:
			if (numwords < 13)
				return FALSE;

			/* extract the matrix from the raw data */
			zeus_matrix[0][0] = convert_tms3203x_fp_to_float(data[dataoffs + 1]);
			zeus_matrix[0][1] = convert_tms3203x_fp_to_float(data[dataoffs + 2]);
			zeus_matrix[0][2] = convert_tms3203x_fp_to_float(data[dataoffs + 3]);
			zeus_matrix[1][0] = convert_tms3203x_fp_to_float(data[dataoffs + 4]);
			zeus_matrix[1][1] = convert_tms3203x_fp_to_float(data[dataoffs + 5]);
			zeus_matrix[1][2] = convert_tms3203x_fp_to_float(data[dataoffs + 6]);
			zeus_matrix[2][0] = convert_tms3203x_fp_to_float(data[dataoffs + 7]);
			zeus_matrix[2][1] = convert_tms3203x_fp_to_float(data[dataoffs + 8]);
			zeus_matrix[2][2] = convert_tms3203x_fp_to_float(data[dataoffs + 9]);

			/* extract the translation point from the raw data */
			zeus_point[0] = convert_tms3203x_fp_to_float(data[dataoffs + 10]);
			zeus_point[1] = convert_tms3203x_fp_to_float(data[dataoffs + 11]);
			zeus_point[2] = convert_tms3203x_fp_to_float(data[dataoffs + 12]);

			if (log_fifo)
			{
				log_fifo_command(data, numwords, "");
				logerror("\n\t\tmatrix ( %8.2f %8.2f %8.2f ) ( %8.2f %8.2f %8.2f ) ( %8.2f %8.2f %8.2f )\n\t\tvector %8.2f %8.2f %8.5f\n",
					zeus_matrix[0][0], zeus_matrix[0][1], zeus_matrix[0][2],
					zeus_matrix[1][0], zeus_matrix[1][1], zeus_matrix[1][2],
					zeus_matrix[2][0], zeus_matrix[2][1], zeus_matrix[2][2],
					zeus_point[0],
					zeus_point[1],
					zeus_point[2]);
			}
			break;

		/* 0x15: set point only (thegrid) */
		/* 0x16: set point only (crusnexo) */
		case 0x15:
		case 0x16:
			if (numwords < 4)
				return FALSE;

			/* extract the translation point from the raw data */
			zeus_point[0] = convert_tms3203x_fp_to_float(data[1]);
			zeus_point[1] = convert_tms3203x_fp_to_float(data[2]);
			zeus_point[2] = convert_tms3203x_fp_to_float(data[3]);

			if (log_fifo)
			{
				log_fifo_command(data, numwords, "");
				logerror("\n\t\tvector %8.2f %8.2f %8.5f\n",
					zeus_point[0],
					zeus_point[1],
					zeus_point[2]);
			}
			break;

		/* 0x1c: */
		case 0x1c:
			if (numwords < 4)
				return FALSE;
			if (log_fifo)
			{
				log_fifo_command(data, numwords, " -- unknown control + hack clear screen\n");
				logerror("\t\tvector %8.2f %8.2f %8.5f\n",
					convert_tms3203x_fp_to_float(data[1]),
					convert_tms3203x_fp_to_float(data[2]),
					convert_tms3203x_fp_to_float(data[3]));

				/* extract the translation point from the raw data */
				zeus_point2[0] = convert_tms3203x_fp_to_float(data[1]);
				zeus_point2[1] = convert_tms3203x_fp_to_float(data[2]);
				zeus_point2[2] = convert_tms3203x_fp_to_float(data[3]);
			}
			{
				/* not right -- just a hack */
				int x, y;
				for (y = zeus_cliprect.min_y; y <= zeus_cliprect.max_y; y++)
					for (x = zeus_cliprect.min_x; x <= zeus_cliprect.max_x; x++)
						waveram_plot_depth(y, x, 0, 0x7fff);
			}
			break;

		/* 0x23: render model in waveram (thegrid) */
		/* 0x24: render model in waveram (crusnexo) */
		case 0x23:
		case 0x24:
			if (numwords < 2)
				return FALSE;
			if (log_fifo)
				log_fifo_command(data, numwords, "");
			zeus_draw_model(machine, data[1], data[0] & 0xffff, log_fifo);
			break;

		/* 0x31: sync pipeline? (thegrid) */
		/* 0x32: sync pipeline? (crusnexo) */
		case 0x31:
		case 0x32:
			if (log_fifo)
				log_fifo_command(data, numwords, "\n");
			zeus_quad_size = 10;
			break;

		/* 0x38: direct render quad (crusnexo) */
		case 0x38:
			if (numwords < 12)
				return FALSE;
			if (log_fifo)
				log_fifo_command(data, numwords, "");
			break;

		/* 0x40: ???? */
		case 0x40:
			if (log_fifo)
				log_fifo_command(data, numwords, "\n");
			break;

		default:
			if (data[0] != 0x2c0)
			{
				printf("Unknown command %08X\n", data[0]);
				if (log_fifo)
					log_fifo_command(data, numwords, "\n");
			}
			break;
	}
	return TRUE;
}



/*************************************
 *
 *  Draw a model in waveram
 *
 *************************************/

static void zeus_draw_model(running_machine *machine, UINT32 baseaddr, UINT16 count, int logit)
{
	UINT32 databuffer[32];
	int databufcount = 0;
	int model_done = FALSE;
	UINT32 texoffs = 0;
	int quadsize = zeus_quad_size;

	if (logit)
		logerror(" -- model @ %08X, len %04X\n", baseaddr, count);

	if (count > 0x1000)
		fatalerror("Extreme count\n");

	while (baseaddr != 0 && !model_done)
	{
		const void *base = waveram0_ptr_from_expanded_addr(baseaddr);
		int curoffs;

		/* reset the objdata address */
		baseaddr = 0;

		/* loop until we run out of data */
		for (curoffs = 0; curoffs <= count; curoffs++)
		{
			int countneeded = 2;
			UINT8 cmd;

			/* accumulate 2 words of data */
			databuffer[databufcount++] = WAVERAM_READ32(base, curoffs * 2 + 0);
			databuffer[databufcount++] = WAVERAM_READ32(base, curoffs * 2 + 1);

			/* if this is enough, process the command */
			cmd = databuffer[0] >> 24;
			if (cmd == 0x38)
				countneeded = quadsize;
			if (databufcount == countneeded)
			{
				/* handle logging of the command */
				if (logit)
				{
					int offs;
					logerror("\t");
					for (offs = 0; offs < databufcount; offs++)
						logerror("%08X ", databuffer[offs]);
					logerror("-- ");
				}

				/* handle the command */
				switch (cmd)
				{
					case 0x21:	/* thegrid */
					case 0x22:	/* crusnexo */
						if (((databuffer[0] >> 16) & 0xff) == 0x9b)
						{
							texoffs = databuffer[1];
							if (logit)
								logerror("texture offset\n");
						}
						else if (logit)
							logerror("unknown offset\n");
						break;

					case 0x31:	/* thegrid */
						if (logit)
							logerror("sync?\n");
						break;

					case 0x35:	/* thegrid */
					case 0x36:	/* crusnexo */
						if (logit)
							logerror("reg32");
						zeus_register32_w(machine, (databuffer[0] >> 16) & 0x7f, databuffer[1], logit);
						break;

					case 0x38:	/* crusnexo/thegrid */
						zeus_draw_quad(machine, databuffer, texoffs, logit);
						break;

					default:
						if (quadsize == 10)
						{
							logerror("Correcting quad size\n");
							quadsize = 14;
						}
						if (logit)
							logerror("unknown model data\n");
						break;
				}

				/* reset the count */
				databufcount = 0;
			}
		}
	}
}



/*************************************
 *
 *  Draw a quad
 *
 *************************************/

static void zeus_draw_quad(running_machine *machine, const UINT32 *databuffer, UINT32 texoffs, int logit)
{
	poly_draw_scanline_func callback;
	poly_extra_data *extra;
	poly_vertex clipvert[8];
	poly_vertex vert[4];
//  float uscale, vscale;
	float maxy, maxx;
//  int val1, val2, texwshift;
	int numverts;
	int i;
//  INT16 normal[3];
//  INT32 rotnormal[3];
	int texmode = texoffs & 0xffff;

	if (logit)
		logerror("quad\n");

if (input_code_pressed(machine, KEYCODE_Q) && (texoffs & 0xffff) == 0x119) return;
if (input_code_pressed(machine, KEYCODE_E) && (texoffs & 0xffff) == 0x01d) return;
if (input_code_pressed(machine, KEYCODE_R) && (texoffs & 0xffff) == 0x11d) return;
if (input_code_pressed(machine, KEYCODE_T) && (texoffs & 0xffff) == 0x05d) return;
if (input_code_pressed(machine, KEYCODE_Y) && (texoffs & 0xffff) == 0x0dd) return;
//if (input_code_pressed(machine, KEYCODE_U) && (texoffs & 0xffff) == 0x119) return;
//if (input_code_pressed(machine, KEYCODE_I) && (texoffs & 0xffff) == 0x119) return;
//if (input_code_pressed(machine, KEYCODE_O) && (texoffs & 0xffff) == 0x119) return;
//if (input_code_pressed(machine, KEYCODE_L) && (texoffs & 0x100)) return;

	callback = render_poly_8bit;

/*
0   38800000
1   x2 | x1
2   v1 | u1
3   y2 | y1
4   v2 | u2
5   z2 | z1
6   v3 | u3
7   v4 | u4
8   ???
9   x4 | x3
10  y4 | y3
11  z4 | z3

In memory:
    +0 = ???
    +1 = set via $05410000/value
    +2 = x1
    +3 = y1
    +4 = z1
    +5 = x2
    +6 = y2
    +7 = z2
    +8 = x3
    +9 = y3
    +10= z3
    +11= x4
    +12= y4
    +13= z4
    +14= uv1
    +15= uv2
    +16= uv3
    +17= uv4
    +18= set via $05200000/$05000000 | (value << 10) (uvoffset?)
    +19= ???


    38810000 00000000 00C7|FF38 FF5E|FF5E 15400154 11400114 00000000 00000000 FF38|00C7 00A3|00A3 -- quad
                      xxxx|xxxx yyyy|yyyy                                     xxxx|xxxx yyyy|yyyy
*/

	/* extract raw x,y,z */
	vert[0].x = (INT16)databuffer[2];
	vert[0].y = (INT16)databuffer[3];
	vert[0].p[0] = (INT16)databuffer[6];
	vert[0].p[1] = (databuffer[1] >> 2) & 0xff;
	vert[0].p[2] = (databuffer[1] >> 18) & 0xff;

	vert[1].x = (INT16)(databuffer[2] >> 16);
	vert[1].y = (INT16)(databuffer[3] >> 16);
	vert[1].p[0] = (INT16)(databuffer[6] >> 16);
	vert[1].p[1] = (databuffer[4] >> 2) & 0xff;
	vert[1].p[2] = (databuffer[4] >> 12) & 0xff;

	vert[2].x = (INT16)databuffer[8];
	vert[2].y = (INT16)databuffer[9];
	vert[2].p[0] = (INT16)databuffer[7];
	vert[2].p[1] = (databuffer[4] >> 22) & 0xff;
	vert[2].p[2] = (databuffer[5] >> 2) & 0xff;

	vert[3].x = (INT16)(databuffer[8] >> 16);
	vert[3].y = (INT16)(databuffer[9] >> 16);
	vert[3].p[0] = (INT16)(databuffer[7] >> 16);
	vert[3].p[1] = (databuffer[5] >> 12) & 0xff;
	vert[3].p[2] = (databuffer[5] >> 22) & 0xff;

/*
    vert[0].x = (INT16)databuffer[1];
    vert[0].y = (INT16)databuffer[3];
    vert[0].p[0] = (INT16)databuffer[5];
    vert[0].p[1] = (UINT16)databuffer[2];
    vert[0].p[2] = (UINT16)(databuffer[2] >> 16);

    vert[1].x = (INT16)(databuffer[1] >> 16);
    vert[1].y = (INT16)(databuffer[3] >> 16);
    vert[1].p[0] = (INT16)(databuffer[5] >> 16);
    vert[1].p[1] = (UINT16)databuffer[4];
    vert[1].p[2] = (UINT16)(databuffer[4] >> 16);

    vert[2].x = (INT16)databuffer[9];
    vert[2].y = (INT16)databuffer[10];
    vert[2].p[0] = (INT16)databuffer[11];
    vert[2].p[1] = (UINT16)databuffer[6];
    vert[2].p[2] = (UINT16)(databuffer[6] >> 16);

    vert[3].x = (INT16)(databuffer[9] >> 16);
    vert[3].y = (INT16)(databuffer[10] >> 16);
    vert[3].p[0] = (INT16)(databuffer[11] >> 16);
    vert[3].p[1] = (UINT16)databuffer[7];
    vert[3].p[2] = (UINT16)(databuffer[7] >> 16);
*/
	for (i = 0; i < 4; i++)
	{
		float x = vert[i].x;
		float y = vert[i].y;
		float z = vert[i].p[0];

		vert[i].x = x * zeus_matrix[0][0] + y * zeus_matrix[0][1] + z * zeus_matrix[0][2] + zeus_point[0];
		vert[i].y = x * zeus_matrix[1][0] + y * zeus_matrix[1][1] + z * zeus_matrix[1][2] + zeus_point[1];
		vert[i].p[0] = x * zeus_matrix[2][0] + y * zeus_matrix[2][1] + z * zeus_matrix[2][2] + zeus_point[2];
		vert[i].p[0] += zbase;
		vert[i].p[2] += texoffs >> 16;
		vert[i].p[1] *= 256.0f;
		vert[i].p[2] *= 256.0f;

		if (logit)
		{
			logerror("\t\t(%f,%f,%f) (%02X,%02X)\n",
					vert[i].x, vert[i].y, vert[i].p[0],
					(int)(vert[i].p[1] / 256.0f), (int)(vert[i].p[2] / 256.0f));
		}
	}

	numverts = poly_zclip_if_less(4, &vert[0], &clipvert[0], 4, 1.0f / 512.0f / 4.0f);
	if (numverts < 3)
		return;

	maxx = maxy = -1000.0f;
	for (i = 0; i < numverts; i++)
	{
// 412.0f here works for crusnexo
		float ooz = 512.0f / clipvert[i].p[0];

//      ooz *= 1.0f / (512.0f * 512.0f);

		clipvert[i].x *= ooz;
		clipvert[i].y *= ooz;
		clipvert[i].x += 256.5f;
		clipvert[i].y += 200.5f;
		clipvert[i].p[0] *= 65536.0f * 16.0f;

		maxx = MAX(maxx, clipvert[i].x);
		maxy = MAX(maxy, clipvert[i].y);
		if (logit)
			logerror("\t\t\tTranslated=(%f,%f)\n", clipvert[i].x, clipvert[i].y);
	}
	for (i = 0; i < numverts; i++)
	{
		if (clipvert[i].x == maxx)
			clipvert[i].x += 0.0005f;
		if (clipvert[i].y == maxy)
			clipvert[i].y += 0.0005f;
	}

	extra = (poly_extra_data *)poly_get_extra_data(poly);
	switch (texmode)
	{
		case 0x01d:		/* crusnexo: RHS of score bar */
		case 0x05d:		/* crusnexo: background, road */
		case 0x0dd:		/* crusnexo: license plate letters */
		case 0x11d:		/* crusnexo: LHS of score bar */
		case 0x15d:		/* crusnexo */
		case 0x85d:		/* crusnexo */
		case 0x95d:		/* crusnexo */
		case 0xc1d:		/* crusnexo */
		case 0xc5d:		/* crusnexo */
			extra->texwidth = 256;
			break;

		case 0x059:		/* crusnexo */
		case 0x0d9:		/* crusnexo */
		case 0x119:		/* crusnexo: license plates */
		case 0x159:		/* crusnexo */
			extra->texwidth = 128;
			break;

		case 0x055:		/* crusnexo */
		case 0x155:		/* crusnexo */
			extra->texwidth = 64;
			break;

		default:
		{
			static UINT8 hits[0x10000];
			if (!hits[(texoffs & 0xffff)])
			{
				hits[(texoffs & 0xffff)] = 1;
				printf("format = %04X\n", (texoffs & 0xffff));
			}
			break;
		}
	}

	extra->solidcolor = 0;//zeusbase[0x00] & 0x7fff;
	extra->zoffset = 0;//zeusbase[0x7e] >> 16;
	extra->alpha = 0;//zeusbase[0x4e];
	extra->transcolor = 0x100;//((databuffer[1] >> 16) & 1) ? 0 : 0x100;
	extra->texbase = WAVERAM_BLOCK0(zeus_texbase);
	extra->palbase = waveram0_ptr_from_expanded_addr(zeusbase[0x41]);

	poly_render_quad_fan(poly, NULL, &zeus_cliprect, callback, 4, numverts, &clipvert[0]);
}



/*************************************
 *
 *  Rasterizers
 *
 *************************************/

static void render_poly_8bit(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	INT32 curz = extent->param[0].start;
	INT32 curu = extent->param[1].start;
	INT32 curv = extent->param[2].start;
//  INT32 curi = extent->param[3].start;
	INT32 dzdx = extent->param[0].dpdx;
	INT32 dudx = extent->param[1].dpdx;
	INT32 dvdx = extent->param[2].dpdx;
//  INT32 didx = extent->param[3].dpdx;
	const void *texbase = extra->texbase;
	const void *palbase = extra->palbase;
	UINT16 transcolor = extra->transcolor;
	int texwidth = extra->texwidth;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT16 *depthptr = WAVERAM_PTRDEPTH(zeus_renderbase, scanline, x);
		INT32 depth = (curz >> 16) + extra->zoffset;
		if (depth > 0x7fff) depth = 0x7fff;
		if (depth >= 0 && depth <= *depthptr)
		{
			int u0 = (curu >> 8);// & (texwidth - 1);
			int v0 = (curv >> 8);// & 255;
			int u1 = (u0 + 1);
			int v1 = (v0 + 1);
			UINT8 texel0 = get_texel_8bit(texbase, v0, u0, texwidth);
			UINT8 texel1 = get_texel_8bit(texbase, v0, u1, texwidth);
			UINT8 texel2 = get_texel_8bit(texbase, v1, u0, texwidth);
			UINT8 texel3 = get_texel_8bit(texbase, v1, u1, texwidth);
			if (texel0 != transcolor)
			{
				rgb_t color0 = WAVERAM_READ16(palbase, texel0);
				rgb_t color1 = WAVERAM_READ16(palbase, texel1);
				rgb_t color2 = WAVERAM_READ16(palbase, texel2);
				rgb_t color3 = WAVERAM_READ16(palbase, texel3);
				rgb_t filtered;
				color0 = ((color0 & 0x7c00) << 9) | ((color0 & 0x3e0) << 6) | ((color0 & 0x1f) << 3);
				color1 = ((color1 & 0x7c00) << 9) | ((color1 & 0x3e0) << 6) | ((color1 & 0x1f) << 3);
				color2 = ((color2 & 0x7c00) << 9) | ((color2 & 0x3e0) << 6) | ((color2 & 0x1f) << 3);
				color3 = ((color3 & 0x7c00) << 9) | ((color3 & 0x3e0) << 6) | ((color3 & 0x1f) << 3);
				filtered = rgb_bilinear_filter(color0, color1, color2, color3, curu, curv);
				WAVERAM_WRITEPIX(zeus_renderbase, scanline, x, filtered);
				*depthptr = depth;
			}
		}

		curz += dzdx;
		curu += dudx;
		curv += dvdx;
//      curi += didx;
	}
}



/*************************************
 *
 *  Debugging tools
 *
 *************************************/

static void log_fifo_command(const UINT32 *data, int numwords, const char *suffix)
{
	int wordnum;

	logerror("Zeus cmd %02X :", data[0] >> 24);
	for (wordnum = 0; wordnum < numwords; wordnum++)
		logerror(" %08X", data[wordnum]);
	logerror("%s", suffix);
}


#if 0
static void log_waveram(UINT32 base, UINT16 length)
{
	static struct
	{
		UINT32 base;
		UINT16 length;
		UINT32 checksum;
	} recent_entries[100];

	UINT32 numoctets = length + 1;
	const void *ptr = waveram0_ptr_from_expanded_addr(base);
	UINT32 checksum = base | ((UINT64)length << 32);
	int foundit = FALSE;
	int i;

	for (i = 0; i < numoctets; i++)
		checksum += WAVERAM_READ32(ptr, i*2) + WAVERAM_READ32(ptr, i*2+1);

	for (i = 0; i < ARRAY_LENGTH(recent_entries); i++)
		if (recent_entries[i].base == base && recent_entries[i].length == length && recent_entries[i].checksum == checksum)
		{
			foundit = TRUE;
			break;
		}

	if (i == ARRAY_LENGTH(recent_entries))
		i--;
	if (i != 0)
	{
		memmove(&recent_entries[1], &recent_entries[0], i * sizeof(recent_entries[0]));
		recent_entries[0].base = base;
		recent_entries[0].length = length;
		recent_entries[0].checksum = checksum;
	}
	if (foundit)
		return;

	for (i = 0; i < numoctets; i++)
		logerror("\t%02X: %08X %08X\n", i, WAVERAM_READ32(ptr, i*2), WAVERAM_READ32(ptr, i*2+1));
}
#endif
