/***************************************************************************

    atarirle.h

    Common RLE-based motion object management functions for early 90's
    Atari raster games.

***************************************************************************/

#ifndef __ATARIRLE__
#define __ATARIRLE__


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* maximum number of motion object processors */
#define ATARIRLE_MAX				1

#define ATARIRLE_PRIORITY_SHIFT		12
#define ATARIRLE_BANK_SHIFT			15
#define ATARIRLE_PRIORITY_MASK		((~0 << ATARIRLE_PRIORITY_SHIFT) & 0xffff)
#define ATARIRLE_DATA_MASK			(ATARIRLE_PRIORITY_MASK ^ 0xffff)

#define ATARIRLE_CONTROL_MOGO		1
#define ATARIRLE_CONTROL_ERASE		2
#define ATARIRLE_CONTROL_FRAME		4

#define ATARIRLE_COMMAND_NOP		0
#define ATARIRLE_COMMAND_DRAW		1
#define ATARIRLE_COMMAND_CHECKSUM	2



/***************************************************************************
    TYPES & STRUCTURES
***************************************************************************/

/* description for an eight-word mask */
typedef struct atarirle_entry atarirle_entry;
struct atarirle_entry
{
	UINT16			data[8];
};

/* description of the motion objects */
typedef struct atarirle_desc atarirle_desc;
struct atarirle_desc
{
	const char *	region;				/* region where the GFX data lives */
	UINT16			spriteramentries;	/* number of entries in sprite RAM */
	UINT16			leftclip;			/* left clip coordinate */
	UINT16			rightclip;			/* right clip coordinate */

	UINT16			palettebase;		/* base palette entry */
	UINT16			maxcolors;			/* maximum number of colors */

	atarirle_entry	codemask;			/* mask for the code index */
	atarirle_entry	colormask;			/* mask for the color */
	atarirle_entry	xposmask;			/* mask for the X position */
	atarirle_entry	yposmask;			/* mask for the Y position */
	atarirle_entry	scalemask;			/* mask for the scale factor */
	atarirle_entry	hflipmask;			/* mask for the horizontal flip */
	atarirle_entry	ordermask;			/* mask for the order */
	atarirle_entry	prioritymask;		/* mask for the priority */
	atarirle_entry	vrammask;			/* mask for the VRAM target */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* setup/shutdown */
void atarirle_init(running_machine *machine, int map, const atarirle_desc *desc);

/* control handlers */
void atarirle_control_w(running_machine *machine, int map, UINT8 bits);
void atarirle_command_w(int map, UINT8 command);
VIDEO_EOF( atarirle );

/* write handlers */
WRITE16_HANDLER( atarirle_0_spriteram_w );
WRITE32_HANDLER( atarirle_0_spriteram32_w );

/* render helpers */
bitmap_t *atarirle_get_vram(int map, int idx);



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern UINT16 *atarirle_0_spriteram;
extern UINT32 *atarirle_0_spriteram32;

#endif
