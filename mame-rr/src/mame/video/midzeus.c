/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/

#include "emu.h"
#include "includes/midzeus.h"
#include "video/poly.h"
#include "video/rgbutil.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define DUMP_WAVE_RAM		0

#define WAVERAM0_WIDTH		512
#define WAVERAM0_HEIGHT		2048

#define WAVERAM1_WIDTH		512
#define WAVERAM1_HEIGHT		512



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

UINT32 *zeusbase;

static poly_manager *poly;
static UINT8 log_fifo;

static UINT32 zeus_fifo[20];
static UINT8 zeus_fifo_words;
static INT16 zeus_matrix[3][3];
static INT32 zeus_point[3];
static INT16 zeus_light[3];
static void *zeus_renderbase;
static UINT32 zeus_palbase;
static int zeus_enable_logging;
static UINT32 zeus_objdata;
static rectangle zeus_cliprect;

static UINT32 *waveram[2];
static int yoffs;
static int texel_width;



/*************************************
 *
 *  Function prototypes
 *
 *************************************/

static void exit_handler(running_machine &machine);

static void zeus_pointer_w(UINT32 which, UINT32 data, int logit);
static void zeus_register16_w(running_machine *machine, offs_t offset, UINT16 data, int logit);
static void zeus_register32_w(running_machine *machine, offs_t offset, UINT32 data, int logit);
static void zeus_register_update(running_machine *machine, offs_t offset);
static int zeus_fifo_process(running_machine *machine, const UINT32 *data, int numwords);
static void zeus_draw_model(running_machine *machine, UINT32 texdata, int logit);
static void zeus_draw_quad(running_machine *machine, const UINT32 *databuffer, UINT32 texdata, int logit);

static void render_poly_4bit(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);
static void render_poly_8bit(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);
static void render_poly_shade(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);
static void render_poly_solid(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);
static void render_poly_solid_fixedz(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);

static void log_fifo_command(const UINT32 *data, int numwords, const char *suffix);
static void log_waveram(UINT32 length_and_base);



/*************************************
 *
 *  Macros
 *
 *************************************/

#define WAVERAM_BLOCK0(blocknum)				((void *)((UINT8 *)waveram[0] + 8 * (blocknum)))
#define WAVERAM_BLOCK1(blocknum)				((void *)((UINT8 *)waveram[1] + 8 * (blocknum)))

#define WAVERAM_PTR8(base, bytenum)				((UINT8 *)(base) + BYTE4_XOR_LE(bytenum))
#define WAVERAM_READ8(base, bytenum)			(*WAVERAM_PTR8(base, bytenum))
#define WAVERAM_WRITE8(base, bytenum, data)		do { *WAVERAM_PTR8(base, bytenum) = (data); } while (0)

#define WAVERAM_PTR16(base, wordnum)			((UINT16 *)(base) + BYTE_XOR_LE(wordnum))
#define WAVERAM_READ16(base, wordnum)			(*WAVERAM_PTR16(base, wordnum))
#define WAVERAM_WRITE16(base, wordnum, data)	do { *WAVERAM_PTR16(base, wordnum) = (data); } while (0)

#define WAVERAM_PTR32(base, dwordnum)			((UINT32 *)(base) + (dwordnum))
#define WAVERAM_READ32(base, dwordnum)			(*WAVERAM_PTR32(base, dwordnum))
#define WAVERAM_WRITE32(base, dwordnum, data)	do { *WAVERAM_PTR32(base, dwordnum) = (data); } while (0)

#define PIXYX_TO_WORDNUM(y, x)					(((y) << 10) | (((x) & 0x1fe) << 1) | ((x) & 1))
#define DEPTHYX_TO_WORDNUM(y, x)				(PIXYX_TO_WORDNUM(y, x) | 2)

#define WAVERAM_PTRPIX(base, y, x)				WAVERAM_PTR16(base, PIXYX_TO_WORDNUM(y, x))
#define WAVERAM_READPIX(base, y, x)				(*WAVERAM_PTRPIX(base, y, x))
#define WAVERAM_WRITEPIX(base, y, x, color)		do { *WAVERAM_PTRPIX(base, y, x) = (color);  } while (0)

#define WAVERAM_PTRDEPTH(base, y, x)			WAVERAM_PTR16(base, DEPTHYX_TO_WORDNUM(y, x))
#define WAVERAM_READDEPTH(base, y, x)			(*WAVERAM_PTRDEPTH(base, y, x))
#define WAVERAM_WRITEDEPTH(base, y, x, color)	do { *WAVERAM_PTRDEPTH(base, y, x) = (color);  } while (0)



/*************************************
 *
 *  Inlines for block addressing
 *
 *************************************/

INLINE void *waveram0_ptr_from_block_addr(UINT32 addr)
{
	UINT32 blocknum = (addr % WAVERAM0_WIDTH) + ((addr >> 12) % WAVERAM0_HEIGHT) * WAVERAM0_WIDTH;
	return WAVERAM_BLOCK0(blocknum);
}

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

INLINE void *waveram0_ptr_from_texture_addr(UINT32 addr, int width)
{
	UINT32 blocknum = (((addr & ~1) * width) / 8) % (WAVERAM0_WIDTH * WAVERAM0_HEIGHT);
	return WAVERAM_BLOCK0(blocknum);
}



/*************************************
 *
 *  Inlines for rendering
 *
 *************************************/

INLINE void waveram_plot(int y, int x, UINT16 color)
{
	if (x >= 0 && x <= zeus_cliprect.max_x && y >= 0 && y < zeus_cliprect.max_y)
		WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
}

INLINE void waveram_plot_depth(int y, int x, UINT16 color, UINT16 depth)
{
	if (x >= 0 && x <= zeus_cliprect.max_x && y >= 0 && y < zeus_cliprect.max_y)
	{
		WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
		WAVERAM_WRITEDEPTH(zeus_renderbase, y, x, depth);
	}
}

INLINE void waveram_plot_check_depth(int y, int x, UINT16 color, UINT16 depth)
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

#ifdef UNUSED_FUNCTION
INLINE void waveram_plot_check_depth_nowrite(int y, int x, UINT16 color, UINT16 depth)
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


INLINE UINT8 get_texel_4bit(const void *base, int y, int x, int width)
{
	UINT32 byteoffs = (y / 2) * (width * 2) + ((x / 8) << 3) + ((y & 1) << 2) + ((x / 2) & 3);
	return (WAVERAM_READ8(base, byteoffs) >> (4 * (x & 1))) & 0x0f;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( midzeus )
{
	int i;

	/* allocate memory for "wave" RAM */
	waveram[0] = auto_alloc_array(machine, UINT32, WAVERAM0_WIDTH * WAVERAM0_HEIGHT * 8/4);
	waveram[1] = auto_alloc_array(machine, UINT32, WAVERAM1_WIDTH * WAVERAM1_HEIGHT * 8/4);

	/* initialize a 5-5-5 palette */
	for (i = 0; i < 32768; i++)
		palette_set_color_rgb(machine, i, pal5bit(i >> 10), pal5bit(i >> 5), pal5bit(i >> 0));

	/* initialize polygon engine */
	poly = poly_alloc(machine, 10000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);

	/* we need to cleanup on exit */
	machine->add_notifier(MACHINE_NOTIFY_EXIT, exit_handler);

	yoffs = 0;
	texel_width = 256;
	zeus_renderbase = waveram[1];

	/* state saving */
	state_save_register_global_array(machine, zeus_fifo);
	state_save_register_global(machine, zeus_fifo_words);
	state_save_register_global_2d_array(machine, zeus_matrix);
	state_save_register_global_array(machine, zeus_point);
	state_save_register_global_array(machine, zeus_light);
	state_save_register_global(machine, zeus_palbase);
	state_save_register_global(machine, zeus_objdata);
	state_save_register_global(machine, zeus_cliprect.min_x);
	state_save_register_global(machine, zeus_cliprect.max_x);
	state_save_register_global(machine, zeus_cliprect.min_y);
	state_save_register_global(machine, zeus_cliprect.max_y);
	state_save_register_global_pointer(machine, waveram[0], WAVERAM0_WIDTH * WAVERAM0_HEIGHT * 8 / sizeof(waveram[0][0]));
	state_save_register_global_pointer(machine, waveram[1], WAVERAM1_WIDTH * WAVERAM1_HEIGHT * 8 / sizeof(waveram[1][0]));
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

	poly_free(poly);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( midzeus )
{
	int x, y;

	poly_wait(poly, "VIDEO_UPDATE");

	/* normal update case */
	if (!input_code_pressed(screen->machine, KEYCODE_W))
	{
		const void *base = waveram1_ptr_from_expanded_addr(zeusbase[0xcc]);
		int xoffs = screen->visible_area().min_x;
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
				dest[x] = WAVERAM_READPIX(base, y, x - xoffs) & 0x7fff;
		}
	}

	/* waveram drawing case */
	else
	{
		const void *base;

		if (input_code_pressed(screen->machine, KEYCODE_DOWN)) yoffs += input_code_pressed(screen->machine, KEYCODE_LSHIFT) ? 0x40 : 1;
		if (input_code_pressed(screen->machine, KEYCODE_UP)) yoffs -= input_code_pressed(screen->machine, KEYCODE_LSHIFT) ? 0x40 : 1;
		if (input_code_pressed(screen->machine, KEYCODE_LEFT) && texel_width > 4) { texel_width >>= 1; while (input_code_pressed(screen->machine, KEYCODE_LEFT)) ; }
		if (input_code_pressed(screen->machine, KEYCODE_RIGHT) && texel_width < 512) { texel_width <<= 1; while (input_code_pressed(screen->machine, KEYCODE_RIGHT)) ; }

		if (yoffs < 0) yoffs = 0;
		base = waveram0_ptr_from_block_addr(yoffs << 12);

		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				UINT8 tex = get_texel_8bit(base, y, x, texel_width);
				dest[x] = (tex << 8) | tex;
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

READ32_HANDLER( zeus_r )
{
	int logit = (offset < 0xb0 || offset > 0xb7);
	UINT32 result = zeusbase[offset & ~1];

	switch (offset & ~1)
	{
		case 0xf0:
			result = space->machine->primary_screen->hpos();
			logit = 0;
			break;

		case 0xf2:
			result = space->machine->primary_screen->vpos();
			logit = 0;
			break;

		case 0xf4:
			result = 6;
			if (space->machine->primary_screen->vblank())
				result |= 0x800;
			logit = 0;
			break;

		case 0xf6:		// status -- they wait for this & 9 == 0
			// value & $9600 must == $9600 to pass Zeus system test
			result = 0x9600;
			if (zeusbase[0xb6] == 0x80040000)
				result |= 1;
			logit = 0;
			break;
	}

	/* 32-bit mode */
	if (zeusbase[0x80] & 0x00020000)
	{
		if (offset & 1)
			result >>= 16;
		if (logit)
		{
			if (offset & 1)
				logerror("%06X:zeus32_r(%02X) = %08X -- unexpected in 32-bit mode\n", cpu_get_pc(space->cpu), offset, result);
			else if (offset != 0xe0)
				logerror("%06X:zeus32_r(%02X) = %08X\n", cpu_get_pc(space->cpu), offset, result);
			else
				logerror("%06X:zeus32_r(%02X) = %08X\n", cpu_get_pc(space->cpu), offset, result);
		}
	}

	/* 16-bit mode */
	else
	{
		if (offset & 1)
			result >>= 16;
		else
			result &= 0xffff;
		if (logit)
			logerror("%06X:zeus16_r(%02X) = %04X\n", cpu_get_pc(space->cpu), offset, result);
	}
	return result;
}



/*************************************
 *
 *  Core write handler
 *
 *************************************/

WRITE32_HANDLER( zeus_w )
{
	int logit = zeus_enable_logging || ((offset < 0xb0 || offset > 0xb7) && (offset < 0xe0 || offset > 0xe1));

	if (logit)
		logerror("%06X:zeus_w", cpu_get_pc(space->cpu));

	/* 32-bit mode */
	if (zeusbase[0x80] & 0x00020000)
		zeus_register32_w(space->machine, offset, data, logit);

	/* 16-bit mode */
	else
		zeus_register16_w(space->machine, offset, data, logit);
}



/*************************************
 *
 *  Handle writes to an internal
 *  pointer register
 *
 *************************************/

static void zeus_pointer_w(UINT32 which, UINT32 data, int logit)
{
	switch (which & 0xffffff)
	{
		case 0x008000:
		case 0x018000:
			if (logit)
				logerror(" -- setptr(objdata)\n");
			zeus_objdata = data;
			break;

		// case 0x00c040: -- set in model data in invasn
		case 0x00c040:
			if (logit)
				logerror(" -- setptr(palbase)\n");
			zeus_palbase = data;
			break;


		// case 0x004040: -- set via FIFO command in mk4 (len=02)

		// case 0x02c0f0: -- set in model data in mk4 (len=0f)
		// case 0x03c0f0: -- set via FIFO command in mk4 (len=00)

		// case 0x02c0e7: -- set via FIFO command in mk4 (len=08)

		// case 0x04c09c: -- set via FIFO command in mk4 (len=08)

		// case 0x05c0a5: -- set via FIFO command in mk4 (len=21)
		// case 0x80c0a5: -- set via FIFO command in mk4 (len=3f)
		// case 0x81c0a5: -- set via FIFO command in mk4 (len=35)
		// case 0x82c0a5: -- set via FIFO command in mk4 (len=41)


		// case 0x00c0f0: -- set via FIFO command in invasn (len=0f)

		// case 0x00c0b0: -- set via FIFO command in invasn (len=3f) -- seems to be the same as c0a5
		// case 0x05c0b0: -- set via FIFO command in invasn (len=21)

		// case 0x00c09c: -- set via FIFO command in invasn (len=06)

		// case 0x00c0a3: -- set via FIFO command in invasn (len=0a)


		default:
			if (logit)
				logerror(" -- setptr(%06X)\n", which & 0xffffff);
			break;
	}

	if (logit)
		log_waveram(data);
}



/*************************************
 *
 *  Handle register writes
 *
 *************************************/

static void zeus_register16_w(running_machine *machine, offs_t offset, UINT16 data, int logit)
{
	/* writes to register $CC need to force a partial update */
	if ((offset & ~1) == 0xcc)
		machine->primary_screen->update_partial(machine->primary_screen->vpos());

	/* write to high part on odd addresses */
	if (offset & 1)
		zeusbase[offset & ~1] = (zeusbase[offset & ~1] & 0x0000ffff) | (data << 16);

	/* write to low part on event addresses */
	else
		zeusbase[offset & ~1] = (zeusbase[offset & ~1] & 0xffff0000) | (data & 0xffff);

	/* log appropriately */
	if (logit)
		logerror("(%02X) = %04X [%08X]\n", offset, data & 0xffff, zeusbase[offset & ~1]);

	/* handle the update */
	if ((offset & 1) == 0)
		zeus_register_update(machine, offset);
}


static void zeus_register32_w(running_machine *machine, offs_t offset, UINT32 data, int logit)
{
	/* writes to register $CC need to force a partial update */
	if ((offset & ~1) == 0xcc)
		machine->primary_screen->update_partial(machine->primary_screen->vpos());

	/* always write to low word? */
	zeusbase[offset & ~1] = data;

	/* log appropriately */
	if (logit)
	{
		if (offset & 1)
			logerror("(%02X) = %08X -- unexpected in 32-bit mode\n", offset, data);
		else if (offset != 0xe0)
			logerror("(%02X) = %08X\n", offset, data);
		else
			logerror("(%02X) = %08X\n", offset, data);
	}

	/* handle the update */
	if ((offset & 1) == 0)
		zeus_register_update(machine, offset);
}



/*************************************
 *
 *  Update state after a register write
 *
 *************************************/

static void zeus_register_update(running_machine *machine, offs_t offset)
{
	/* handle the writes; only trigger on low accesses */
	switch (offset)
	{
		case 0x52:
			zeusbase[0xb2] = zeusbase[0x52];
			break;

		case 0x60:
			/* invasn writes here to execute a command (?) */
			if (zeusbase[0x60] & 1)
			{
				if ((zeusbase[0x80] & 0xffffff) == 0x22FCFF)
				{
					// zeusbase[0x00] = color
					// zeusbase[0x02] = ??? = 0x000C0000
					// zeusbase[0x04] = ??? = 0x00000E01
					// zeusbase[0x06] = ??? = 0xFFFF0030
					// zeusbase[0x08] = vert[0] = (y0 << 16) | x0
					// zeusbase[0x0a] = vert[1] = (y1 << 16) | x1
					// zeusbase[0x0c] = vert[2] = (y2 << 16) | x2
					// zeusbase[0x0e] = vert[3] = (y3 << 16) | x3
					// zeusbase[0x18] = ??? = 0xFFFFFFFF
					// zeusbase[0x1a] = ??? = 0xFFFFFFFF
					// zeusbase[0x1c] = ??? = 0xFFFFFFFF
					// zeusbase[0x1e] = ??? = 0xFFFFFFFF
					// zeusbase[0x20] = ??? = 0x00000000
					// zeusbase[0x22] = ??? = 0x00000000
					// zeusbase[0x24] = ??? = 0x00000000
					// zeusbase[0x26] = ??? = 0x00000000
					// zeusbase[0x40] = ??? = 0x00000000
					// zeusbase[0x42] = ??? = 0x00000000
					// zeusbase[0x44] = ??? = 0x00000000
					// zeusbase[0x46] = ??? = 0x00000000
					// zeusbase[0x4c] = ??? = 0x00808080 (brightness?)
					// zeusbase[0x4e] = ??? = 0x00808080 (brightness?)
					poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
					poly_vertex vert[4];

					vert[0].x = (INT16)zeusbase[0x08];
					vert[0].y = (INT16)(zeusbase[0x08] >> 16);
					vert[1].x = (INT16)zeusbase[0x0a];
					vert[1].y = (INT16)(zeusbase[0x0a] >> 16);
					vert[2].x = (INT16)zeusbase[0x0c];
					vert[2].y = (INT16)(zeusbase[0x0c] >> 16);
					vert[3].x = (INT16)zeusbase[0x0e];
					vert[3].y = (INT16)(zeusbase[0x0e] >> 16);

					extra->solidcolor = zeusbase[0x00];
					extra->zoffset = 0x7fff;

					poly_render_quad(poly, NULL, &zeus_cliprect, render_poly_solid_fixedz, 0, &vert[0], &vert[1], &vert[2], &vert[3]);
					poly_wait(poly, "Normal");
				}
				else
					logerror("Execute unknown command\n");
			}
			break;

		case 0x70:
			zeus_point[0] = zeusbase[0x70] << 16;
			break;

		case 0x72:
			zeus_point[1] = zeusbase[0x72] << 16;
			break;

		case 0x74:
			zeus_point[2] = zeusbase[0x74] << 16;
			break;

		case 0x80:
			/* this bit enables the "FIFO empty" IRQ; since our virtual FIFO is always empty,
                we simply assert immediately if this is enabled. invasn needs this for proper
                operations */
			if (zeusbase[0x80] & 0x02000000)
				cputag_set_input_line(machine, "maincpu", 2, ASSERT_LINE);
			else
				cputag_set_input_line(machine, "maincpu", 2, CLEAR_LINE);
			break;

		case 0x84:
			/* MK4: Written in tandem with 0xcc */
			/* MK4: Writes either 0x80 (and 0x000000 to 0xcc) or 0x00 (and 0x800000 to 0xcc) */
			zeus_renderbase = waveram1_ptr_from_expanded_addr(zeusbase[0x84] << 16);
			break;

		case 0xb0:
		case 0xb2:
			if ((zeusbase[0xb6] >> 16) != 0)
			{
				if ((offset == 0xb0 && (zeusbase[0xb6] & 0x02000000) == 0) ||
					(offset == 0xb2 && (zeusbase[0xb6] & 0x02000000) != 0))
				{
					void *dest;

					if (zeusbase[0xb6] & 0x80000000)
						dest = waveram1_ptr_from_expanded_addr(zeusbase[0xb4]);
					else
						dest = waveram0_ptr_from_expanded_addr(zeusbase[0xb4]);

					if (zeusbase[0xb6] & 0x00100000)
						WAVERAM_WRITE16(dest, 0, zeusbase[0xb0]);
					if (zeusbase[0xb6] & 0x00200000)
						WAVERAM_WRITE16(dest, 1, zeusbase[0xb0] >> 16);
					if (zeusbase[0xb6] & 0x00400000)
						WAVERAM_WRITE16(dest, 2, zeusbase[0xb2]);
					if (zeusbase[0xb6] & 0x00800000)
						WAVERAM_WRITE16(dest, 3, zeusbase[0xb2] >> 16);
					if (zeusbase[0xb6] & 0x00020000)
						zeusbase[0xb4]++;
				}
			}
			break;

		case 0xb4:
			if (zeusbase[0xb6] & 0x00010000)
			{
				const UINT32 *src;

				if (zeusbase[0xb6] & 0x80000000)
					src = (const UINT32 *)waveram1_ptr_from_expanded_addr(zeusbase[0xb4]);
				else
					src = (const UINT32 *)waveram0_ptr_from_expanded_addr(zeusbase[0xb4]);

				poly_wait(poly, "vram_read");
				zeusbase[0xb0] = WAVERAM_READ32(src, 0);
				zeusbase[0xb2] = WAVERAM_READ32(src, 1);
			}
			break;

		case 0xc0:
		case 0xc2:
		case 0xc4:
		case 0xc6:
		case 0xc8:
		case 0xca:
			machine->primary_screen->update_partial(machine->primary_screen->vpos());
			{
				int vtotal = zeusbase[0xca] >> 16;
				int htotal = zeusbase[0xc6] >> 16;
				rectangle visarea;

				visarea.min_x = zeusbase[0xc6] & 0xffff;
				visarea.max_x = htotal - 3;
				visarea.min_y = 0;
				visarea.max_y = zeusbase[0xc8] & 0xffff;
				if (htotal > 0 && vtotal > 0 && visarea.min_x < visarea.max_x && visarea.max_y < vtotal)
				{
					machine->primary_screen->configure(htotal, vtotal, visarea, HZ_TO_ATTOSECONDS((double)MIDZEUS_VIDEO_CLOCK / 8.0 / (htotal * vtotal)));
					zeus_cliprect = visarea;
					zeus_cliprect.max_x -= zeus_cliprect.min_x;
					zeus_cliprect.min_x = 0;
				}
			}
			break;

		case 0xcc:
			machine->primary_screen->update_partial(machine->primary_screen->vpos());
			log_fifo = input_code_pressed(machine, KEYCODE_L);
			break;

		case 0xe0:
			zeus_fifo[zeus_fifo_words++] = zeusbase[0xe0];
			if (zeus_fifo_process(machine, zeus_fifo, zeus_fifo_words))
				zeus_fifo_words = 0;
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
	/* handle logging */
	switch (data[0] >> 24)
	{
		/* 0x00/0x01: set pointer */
		/* in model data, this is 0x0C */
		case 0x00:
		case 0x01:
			if (numwords < 2 && data[0] != 0)
				return FALSE;
			if (log_fifo)
				log_fifo_command(data, numwords, "");
			zeus_pointer_w(data[0] & 0xffffff, data[1], log_fifo);
			break;

		/* 0x13: render model based on previously set information */
		case 0x13:	/* invasn */
			if (log_fifo)
				log_fifo_command(data, numwords, "");
			zeus_draw_model(machine, (zeusbase[0x06] << 16), log_fifo);
			break;

		/* 0x17: write 16-bit value to low registers */
		case 0x17:
			if (log_fifo)
				log_fifo_command(data, numwords, " -- reg16");
			zeus_register16_w(machine, (data[0] >> 16) & 0x7f, data[0], log_fifo);
			break;

		/* 0x18: write 32-bit value to low registers */
		/* in model data, this is 0x19 */
		case 0x18:
			if (numwords < 2)
				return FALSE;
			if (log_fifo)
				log_fifo_command(data, numwords, " -- reg32");
			zeus_register32_w(machine, (data[0] >> 16) & 0x7f, data[1], log_fifo);
			break;

		/* 0x1A: sync pipeline(?) */
		case 0x1a:
			if (log_fifo)
				log_fifo_command(data, numwords, " -- sync\n");
			break;

		/* 0x1C: write matrix and translation vector */
		case 0x1c:

			/* single matrix form */
			if ((data[0] & 0xffff) != 0x7fff)
			{
				/* requires 8 words total */
				if (numwords < 8)
					return FALSE;
				if (log_fifo)
				{
					log_fifo_command(data, numwords, "");
					logerror("\n\t\tmatrix ( %04X %04X %04X ) ( %04X %04X %04X ) ( %04X %04X %04X )\n\t\tvector %8.2f %8.2f %8.5f\n",
						data[2] & 0xffff,	data[2] >> 16,		data[0] & 0xffff,
						data[3] & 0xffff,	data[3] >> 16,		data[1] >> 16,
						data[4] & 0xffff,	data[4] >> 16,		data[1] & 0xffff,
						(float)(INT32)data[5] * (1.0f / 65536.0f),
						(float)(INT32)data[6] * (1.0f / 65536.0f),
						(float)(INT32)data[7] * (1.0f / (65536.0f * 512.0f)));
				}

				/* extract the matrix from the raw data */
				zeus_matrix[0][0] = data[2];	zeus_matrix[0][1] = data[2] >> 16;	zeus_matrix[0][2] = data[0];
				zeus_matrix[1][0] = data[3];	zeus_matrix[1][1] = data[3] >> 16;	zeus_matrix[1][2] = data[1] >> 16;
				zeus_matrix[2][0] = data[4];	zeus_matrix[2][1] = data[4] >> 16;	zeus_matrix[2][2] = data[1];

				/* extract the translation point from the raw data */
				zeus_point[0] = data[5];
				zeus_point[1] = data[6];
				zeus_point[2] = data[7];
			}

			/* double matrix form */
			else
			{
				INT16 matrix1[3][3];
				INT16 matrix2[3][3];

				/* requires 13 words total */
				if (numwords < 13)
					return FALSE;
				if (log_fifo)
				{
					log_fifo_command(data, numwords, "");
					logerror("\n\t\tmatrix ( %04X %04X %04X ) ( %04X %04X %04X ) ( %04X %04X %04X )\n\t\tmatrix ( %04X %04X %04X ) ( %04X %04X %04X ) ( %04X %04X %04X )\n\t\tvector %8.2f %8.2f %8.5f\n",
						data[4] & 0xffff,	data[4] >> 16,		data[5] >> 16,
						data[8] & 0xffff,	data[8] >> 16,		data[6] >> 16,
						data[9] & 0xffff,	data[9] >> 16,		data[7] >> 16,
						data[1] & 0xffff,	data[2] & 0xffff,	data[3] & 0xffff,
						data[1] >> 16,		data[2] >> 16,		data[3] >> 16,
						data[5] & 0xffff,	data[6] & 0xffff,	data[7] & 0xffff,
						(float)(INT32)data[10] * (1.0f / 65536.0f),
						(float)(INT32)data[11] * (1.0f / 65536.0f),
						(float)(INT32)data[12] * (1.0f / (65536.0f * 512.0f)));
				}

				/* extract the first matrix from the raw data */
				matrix1[0][0] = data[4];		matrix1[0][1] = data[4] >> 16;	matrix1[0][2] = data[5] >> 16;
				matrix1[1][0] = data[8];		matrix1[1][1] = data[8] >> 16;	matrix1[1][2] = data[6] >> 16;
				matrix1[2][0] = data[9];		matrix1[2][1] = data[9] >> 16;	matrix1[2][2] = data[7] >> 16;

				/* extract the second matrix from the raw data */
				matrix2[0][0] = data[1];		matrix2[0][1] = data[2];		matrix2[0][2] = data[3];
				matrix2[1][0] = data[1] >> 16;	matrix2[1][1] = data[2] >> 16;	matrix2[1][2] = data[3] >> 16;
				matrix2[2][0] = data[5];		matrix2[2][1] = data[6];		matrix2[2][2] = data[7];

				/* multiply them together to get the final matrix */
				zeus_matrix[0][0] = ((INT64)(matrix1[0][0] * matrix2[0][0]) + (INT64)(matrix1[0][1] * matrix2[1][0]) + (INT64)(matrix1[0][2] * matrix2[2][0])) >> 16;
				zeus_matrix[0][1] = ((INT64)(matrix1[0][0] * matrix2[0][1]) + (INT64)(matrix1[0][1] * matrix2[1][1]) + (INT64)(matrix1[0][2] * matrix2[2][1])) >> 16;
				zeus_matrix[0][2] = ((INT64)(matrix1[0][0] * matrix2[0][2]) + (INT64)(matrix1[0][1] * matrix2[1][2]) + (INT64)(matrix1[0][2] * matrix2[2][2])) >> 16;
				zeus_matrix[1][0] = ((INT64)(matrix1[1][0] * matrix2[0][0]) + (INT64)(matrix1[1][1] * matrix2[1][0]) + (INT64)(matrix1[1][2] * matrix2[2][0])) >> 16;
				zeus_matrix[1][1] = ((INT64)(matrix1[1][0] * matrix2[0][1]) + (INT64)(matrix1[1][1] * matrix2[1][1]) + (INT64)(matrix1[1][2] * matrix2[2][1])) >> 16;
				zeus_matrix[1][2] = ((INT64)(matrix1[1][0] * matrix2[0][2]) + (INT64)(matrix1[1][1] * matrix2[1][2]) + (INT64)(matrix1[1][2] * matrix2[2][2])) >> 16;
				zeus_matrix[2][0] = ((INT64)(matrix1[2][0] * matrix2[0][0]) + (INT64)(matrix1[2][1] * matrix2[1][0]) + (INT64)(matrix1[2][2] * matrix2[2][0])) >> 16;
				zeus_matrix[2][1] = ((INT64)(matrix1[2][0] * matrix2[0][1]) + (INT64)(matrix1[2][1] * matrix2[1][1]) + (INT64)(matrix1[2][2] * matrix2[2][1])) >> 16;
				zeus_matrix[2][2] = ((INT64)(matrix1[2][0] * matrix2[0][2]) + (INT64)(matrix1[2][1] * matrix2[1][2]) + (INT64)(matrix1[2][2] * matrix2[2][2])) >> 16;

				/* extract the translation point from the raw data */
				zeus_point[0] = data[10];
				zeus_point[1] = data[11];
				zeus_point[2] = data[12];
			}
			break;

		/* 0x23: some additional X,Y,Z coordinates */
		/* 0x2e: same for invasn */
		case 0x23:
		case 0x2e:
			if (numwords < 2)
				return FALSE;
			if (log_fifo)
			{
				log_fifo_command(data, numwords, "");
				logerror(" -- additional xyz = %d,%d,%d\n", (INT16)data[0], (INT16)(data[1] >> 16), (INT16)data[1]);

				/* guessing this might be a light source? */
				zeus_light[0] = (INT16)data[0];
				zeus_light[1] = (INT16)(data[1] >> 16);
				zeus_light[2] = (INT16)data[1];
			}
			break;

		/* 0x25: display control? */
		/* 0x30: same for invasn */
		case 0x25:
		case 0x30:
			if (numwords < 4 || ((data[0] & 0x808000) && numwords < 10))
				return FALSE;
			if (log_fifo)
				log_fifo_command(data, numwords, " -- unknown control + hack clear screen\n");
			if ((data[0] & 0xffff7f) == 0)
			{
				/* not right -- just a hack */
				int x, y;
				for (y = zeus_cliprect.min_y; y <= zeus_cliprect.max_y; y++)
					for (x = zeus_cliprect.min_x; x <= zeus_cliprect.max_x; x++)
						waveram_plot_depth(y, x, 0, 0x7fff);
			}
			break;

		/* 0x2d: unknown - invasn */
		/* 0x70: same for mk4 */
		case 0x2d:
		case 0x70:
			if (log_fifo)
				log_fifo_command(data, numwords, "\n");
			break;

		/* 0x67: render model with inline texture info */
		case 0x67:
			if (numwords < 3)
				return FALSE;
			if (log_fifo)
				log_fifo_command(data, numwords, "");
			zeus_objdata = data[1];
			zeus_draw_model(machine, data[2], log_fifo);
			break;

		default:
			printf("Unknown command %08X\n", data[0]);
			if (log_fifo)
				log_fifo_command(data, numwords, "\n");
			break;
	}
	return TRUE;
}



/*************************************
 *
 *  Draw a model in waveram
 *
 *************************************/

static void zeus_draw_model(running_machine *machine, UINT32 texdata, int logit)
{
	UINT32 databuffer[32];
	int databufcount = 0;
	int model_done = FALSE;

	if (logit)
		logerror(" -- model @ %08X\n", zeus_objdata);

	while (zeus_objdata != 0 && !model_done)
	{
		const void *base = waveram0_ptr_from_block_addr(zeus_objdata);
		int count = zeus_objdata >> 24;
		int curoffs;

		/* reset the objdata address */
		zeus_objdata = 0;

		/* loop until we run out of data */
		for (curoffs = 0; curoffs <= count; curoffs++)
		{
			int countneeded;
			UINT8 cmd;

			/* accumulate 2 words of data */
			databuffer[databufcount++] = WAVERAM_READ32(base, curoffs * 2 + 0);
			databuffer[databufcount++] = WAVERAM_READ32(base, curoffs * 2 + 1);

			/* if this is enough, process the command */
			cmd = databuffer[0] >> 24;
			countneeded = (cmd == 0x25 || cmd == 0x30) ? 14 : 2;
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
					case 0x08:
						if (logit)
							logerror("end of model\n");
						model_done = TRUE;
						break;

					case 0x0c:	/* mk4/invasn */
						zeus_pointer_w(databuffer[0] & 0xffffff, databuffer[1], logit);
						break;

					case 0x17:	/* mk4 */
						if (logit)
							logerror("reg16");
						zeus_register16_w(machine, (databuffer[0] >> 16) & 0x7f, databuffer[0], logit);
						if (((databuffer[0] >> 16) & 0x7f) == 0x06)
							texdata = (texdata & 0xffff) | (zeusbase[0x06] << 16);
						break;

					case 0x19:	/* invasn */
						if (logit)
							logerror("reg32");
						zeus_register32_w(machine, (databuffer[0] >> 16) & 0x7f, databuffer[1], logit);
						if (((databuffer[0] >> 16) & 0x7f) == 0x06)
							texdata = (texdata & 0xffff) | (zeusbase[0x06] << 16);
						break;

					case 0x25:	/* mk4 */
					case 0x30:	/* invasn */
						zeus_draw_quad(machine, databuffer, texdata, logit);
						break;

					default:
						if (logit)
							logerror("unknown\n");
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

static void zeus_draw_quad(running_machine *machine, const UINT32 *databuffer, UINT32 texdata, int logit)
{
	poly_draw_scanline_func callback;
	poly_extra_data *extra;
	poly_vertex clipvert[8];
	poly_vertex vert[4];
	float uscale, vscale;
	float maxy, maxx;
	int val1, val2, texwshift;
	int numverts;
	int i;
	INT16 normal[3];
	INT32 rotnormal[3];

/* look for interesting data patterns  */
if (
	(databuffer[1] & 0xffffffff) != 0x200c0000 && /* mk4 sometimes */
	(databuffer[1] & 0xfffe0000) != 0x21000000 && /* most of mk4 */
	(databuffer[1] & 0xffffffff) != 0x008c0000 && /* invasn */
	(databuffer[1] & 0xfffeffff) != 0x028c0000 && /* invasn */
	(databuffer[1] & 0xfffe0000) != 0x21800000 && /* invasn */
	(databuffer[1] & 0xfffe0000) != 0x23800000 && /* invasn */
	1)
	printf("zeus_draw_quad: databuffer[1] = %08X\n", databuffer[1]);


	/* do a simple backface cull; not sure if the hardware does it, but I see no other
       reason for a polygon normal here */

	/* extract the polygon normal */
	normal[0] = (INT8)(databuffer[0] >> 0);
	normal[1] = (INT8)(databuffer[0] >> 8);
	normal[2] = (INT8)(databuffer[0] >> 16);

	/* rotate the normal into camera view; we only need the Z coordinate */
	rotnormal[2] = normal[0] * zeus_matrix[2][0] + normal[1] * zeus_matrix[2][1] + normal[2] * zeus_matrix[2][2];

	/* if we're pointing away from the camera, toss */
	if (rotnormal[2] > 0)
	{
		if (logit)
			logerror("quad (culled %08X)\n", rotnormal[2]);
//      if (input_code_pressed(machine, KEYCODE_COMMA))
//          return;
	}

	if (logit)
		logerror("quad\n");

	texdata = (texdata & 0xffff0000) | ((texdata + databuffer[1]) & 0xffff);

	val1 = ((texdata >> 10) & 0x3f0000) | (texdata & 0xffff);
	val2 = (texdata >> 16) & 0x3ff;
	texwshift = (val2 >> 6) & 7;

//if (input_code_pressed(machine, KEYCODE_Z) && (val2 & 0x01)) return;
//if (input_code_pressed(machine, KEYCODE_X) && (val2 & 0x02)) return;
//if (input_code_pressed(machine, KEYCODE_C) && (val2 & 0x04)) return;
//if (input_code_pressed(machine, KEYCODE_V) && (val2 & 0x08)) return;
//if (input_code_pressed(machine, KEYCODE_B) && (val2 & 0x10)) return;
//if (input_code_pressed(machine, KEYCODE_N) && (val2 & 0x20)) return;
//if (input_code_pressed(machine, KEYCODE_M) && (val2 & 0x200)) return;

	uscale = (8 >> ((zeusbase[0x04] >> 4) & 3)) * 0.125f * 256.0f;
	vscale = (8 >> ((zeusbase[0x04] >> 6) & 3)) * 0.125f * 256.0f;

	if ((databuffer[1] & 0x000c0000) == 0x000c0000)
		callback = render_poly_solid;
	else if (val2 == 0x182)
		callback = render_poly_shade;
	else if (val2 & 0x20)
		callback = render_poly_8bit;
	else
		callback = render_poly_4bit;

	for (i = 0; i < 4; i++)
	{
		UINT32 ixy = databuffer[2 + i*2];
		UINT32 iuvz = databuffer[3 + i*2];
		UINT32 inormal = databuffer[10 + i];
		INT32 xo = (INT16)ixy;
		INT32 yo = (INT16)(ixy >> 16);
		INT32 zo = (INT16)iuvz;
		INT32 xn = (INT32)(inormal <<  2) >> 20;
		INT32 yn = (INT32)(inormal << 12) >> 20;
		INT32 zn = (INT32)(inormal << 22) >> 20;
		UINT8 u = iuvz >> 16;
		UINT8 v = iuvz >> 24;
		INT32 dotnormal;
		INT64 x, y, z;

		x = (INT64)(xo * zeus_matrix[0][0]) + (INT64)(yo * zeus_matrix[0][1]) + (INT64)(zo * zeus_matrix[0][2]) + zeus_point[0];
		y = (INT64)(xo * zeus_matrix[1][0]) + (INT64)(yo * zeus_matrix[1][1]) + (INT64)(zo * zeus_matrix[1][2]) + zeus_point[1];
		z = (INT64)(xo * zeus_matrix[2][0]) + (INT64)(yo * zeus_matrix[2][1]) + (INT64)(zo * zeus_matrix[2][2]) + zeus_point[2];

		rotnormal[0] = ((INT64)(xn * zeus_matrix[0][0]) + (INT64)(yn * zeus_matrix[0][1]) + (INT64)(zn * zeus_matrix[0][2])) >> 14;
		rotnormal[1] = ((INT64)(xn * zeus_matrix[1][0]) + (INT64)(yn * zeus_matrix[1][1]) + (INT64)(zn * zeus_matrix[1][2])) >> 14;
		rotnormal[2] = ((INT64)(xn * zeus_matrix[2][0]) + (INT64)(yn * zeus_matrix[2][1]) + (INT64)(zn * zeus_matrix[2][2])) >> 14;

		dotnormal = rotnormal[0] * ((x >> 16) + zeus_light[0]) + rotnormal[1] * ((y >> 16) + zeus_light[1]) + rotnormal[2] * ((z >> 16) + zeus_light[2]);

		vert[i].x = x;
		vert[i].y = y;
		vert[i].p[0] = z;
		vert[i].p[1] = u * uscale;
		vert[i].p[2] = v * vscale;
		vert[i].p[3] = dotnormal;

		if (logit)
		{
			logerror("\t\t(%f,%f,%f) (%02X,%02X) (%03X,%03X,%03X) dot=%08X\n",
					vert[i].x * (1.0f / 65536.0f), vert[i].y * (1.0f / 65536.0f), vert[i].p[0] * (1.0f / 65536.0f),
					(int)(vert[i].p[1] / 256.0f), (int)(vert[i].p[2] / 256.0f),
					(databuffer[10 + i] >> 20) & 0x3ff, (databuffer[10 + i] >> 10) & 0x3ff, (databuffer[10 + i] >> 0) & 0x3ff,
					dotnormal);
		}
	}

	numverts = poly_zclip_if_less(4, &vert[0], &clipvert[0], 4, 512.0f);
	if (numverts < 3)
		return;

	maxx = maxy = -1000.0f;
	for (i = 0; i < numverts; i++)
	{
		float ooz = 512.0f / clipvert[i].p[0];

		clipvert[i].x *= ooz;
		clipvert[i].y *= ooz;
		clipvert[i].x += 200.5f;
		clipvert[i].y += 128.5f;

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
	extra->solidcolor = zeusbase[0x00] & 0x7fff;
	extra->zoffset = zeusbase[0x7e] >> 16;
	extra->alpha = zeusbase[0x4e];
	extra->transcolor = ((databuffer[1] >> 16) & 1) ? 0 : 0x100;
	extra->texwidth = 512 >> texwshift;
	extra->texbase = waveram0_ptr_from_texture_addr(val1, extra->texwidth);
	extra->palbase = waveram0_ptr_from_block_addr(zeus_palbase);

	poly_render_quad_fan(poly, NULL, &zeus_cliprect, callback, 4, numverts, &clipvert[0]);
}



/*************************************
 *
 *  Rasterizers
 *
 *************************************/

static void render_poly_4bit(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	INT32 curz = extent->param[0].start;
	INT32 curu = extent->param[1].start;
	INT32 curv = extent->param[2].start;
	INT32 curi = extent->param[3].start;
	INT32 dzdx = extent->param[0].dpdx;
	INT32 dudx = extent->param[1].dpdx;
	INT32 dvdx = extent->param[2].dpdx;
	INT32 didx = extent->param[3].dpdx;
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
			UINT8 texel0 = get_texel_4bit(texbase, v0, u0, texwidth);
			UINT8 texel1 = get_texel_4bit(texbase, v0, u1, texwidth);
			UINT8 texel2 = get_texel_4bit(texbase, v1, u0, texwidth);
			UINT8 texel3 = get_texel_4bit(texbase, v1, u1, texwidth);
			if (texel0 != transcolor)
			{
				rgb_t color0 = WAVERAM_READ16(palbase, texel0);
				rgb_t color1 = WAVERAM_READ16(palbase, texel1);
				rgb_t color2 = WAVERAM_READ16(palbase, texel2);
				rgb_t color3 = WAVERAM_READ16(palbase, texel3);
				rgb_t filtered;
				color0 = ((color0 & 0x7fe0) << 6) | (color0 & 0x1f);
				color1 = ((color1 & 0x7fe0) << 6) | (color1 & 0x1f);
				color2 = ((color2 & 0x7fe0) << 6) | (color2 & 0x1f);
				color3 = ((color3 & 0x7fe0) << 6) | (color3 & 0x1f);
				filtered = rgb_bilinear_filter(color0, color1, color2, color3, curu, curv);
				WAVERAM_WRITEPIX(zeus_renderbase, scanline, x, ((filtered >> 6) & 0x7fe0) | (filtered & 0x1f));
				*depthptr = depth;
			}
		}

		curz += dzdx;
		curu += dudx;
		curv += dvdx;
		curi += didx;
	}
}


static void render_poly_8bit(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	INT32 curz = extent->param[0].start;
	INT32 curu = extent->param[1].start;
	INT32 curv = extent->param[2].start;
	INT32 curi = extent->param[3].start;
	INT32 dzdx = extent->param[0].dpdx;
	INT32 dudx = extent->param[1].dpdx;
	INT32 dvdx = extent->param[2].dpdx;
	INT32 didx = extent->param[3].dpdx;
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
				color0 = ((color0 & 0x7fe0) << 6) | (color0 & 0x1f);
				color1 = ((color1 & 0x7fe0) << 6) | (color1 & 0x1f);
				color2 = ((color2 & 0x7fe0) << 6) | (color2 & 0x1f);
				color3 = ((color3 & 0x7fe0) << 6) | (color3 & 0x1f);
				filtered = rgb_bilinear_filter(color0, color1, color2, color3, curu, curv);
				WAVERAM_WRITEPIX(zeus_renderbase, scanline, x, ((filtered >> 6) & 0x7fe0) | (filtered & 0x1f));
				*depthptr = depth;
			}
		}

		curz += dzdx;
		curu += dudx;
		curv += dvdx;
		curi += didx;
	}
}


static void render_poly_shade(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		if (x >= 0 && x < 400)
		{
			if (extra->alpha <= 0x80)
			{
				UINT16 *ptr = WAVERAM_PTRPIX(zeus_renderbase, scanline, x);
				UINT16 pix = *ptr;

				*ptr = ((((pix & 0x7c00) * extra->alpha) >> 7) & 0x7c00) |
					  ((((pix & 0x03e0) * extra->alpha) >> 7) & 0x03e0) |
					  ((((pix & 0x001f) * extra->alpha) >> 7) & 0x001f);
			}
			else
			{
				waveram_plot(scanline, x, 0);
			}
		}
	}
}


static void render_poly_solid(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	UINT16 color = extra->solidcolor;
	INT32 curz = (INT32)(extent->param[0].start);
	INT32 curv = extent->param[2].start;
	INT32 dzdx = (INT32)(extent->param[0].dpdx);
	INT32 dvdx = extent->param[2].dpdx;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		INT32 depth = (curz >> 16) + extra->zoffset;
		if (depth > 0x7fff) depth = 0x7fff;
		if (depth >= 0)
		{
//          UINT32 finalcolor = (((color & 0x7c00) * curv) & 0x7c000000) | (((color & 0x03e0) * curv) & 0x03e00000) | (((color & 0x001f) * curv) & 0x001f0000);
//          waveram_plot_check_depth(scanline, x, finalcolor >> 16, depth);
			waveram_plot_check_depth(scanline, x, color, depth);
		}
		curz += dzdx;
		curv += dvdx;
	}
}


static void render_poly_solid_fixedz(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	UINT16 color = extra->solidcolor;
	UINT16 depth = extra->zoffset;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
		waveram_plot_depth(scanline, x, color, depth);
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


static void log_waveram(UINT32 length_and_base)
{
	static struct
	{
		UINT32 lab;
		UINT32 checksum;
	} recent_entries[100];

	UINT32 numoctets = (length_and_base >> 24) + 1;
	const UINT32 *ptr = (const UINT32 *)waveram0_ptr_from_block_addr(length_and_base);
	UINT32 checksum = length_and_base;
	int foundit = FALSE;
	int i;

	for (i = 0; i < numoctets; i++)
		checksum += ptr[i*2] + ptr[i*2+1];

	for (i = 0; i < ARRAY_LENGTH(recent_entries); i++)
		if (recent_entries[i].lab == length_and_base && recent_entries[i].checksum == checksum)
		{
			foundit = TRUE;
			break;
		}

	if (i == ARRAY_LENGTH(recent_entries))
		i--;
	if (i != 0)
	{
		memmove(&recent_entries[1], &recent_entries[0], i * sizeof(recent_entries[0]));
		recent_entries[0].lab = length_and_base;
		recent_entries[0].checksum = checksum;
	}
	if (foundit)
		return;

	for (i = 0; i < numoctets; i++)
		logerror("\t%02X: %08X %08X\n", i, ptr[i*2], ptr[i*2+1]);
}
