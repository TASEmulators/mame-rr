/******************************************************************************
    Atari 400/800

    Video handler

    Juergen Buchmueller, June 1998
******************************************************************************/

#include "emu.h"
#include "includes/atari.h"
#include "video/gtia.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

char atari_frame_message[64+1];
int atari_frame_counter;

/* flag for displaying television artifacts in ANTIC mode F (15) */
static UINT32 tv_artifacts = 0;

/*************************************************************************
 * The priority tables tell which playfield, player or missile colors
 * have precedence about the others, depending on the contents of the
 * "prior" register. There are 64 possible priority selections.
 * The table is here to make it easier to build the 'illegal' priority
 * combinations that produce black or 'ILL' color.
 *************************************************************************/

/*************************************************************************
 * calculate player/missile priorities (GTIA prior at $D00D)
 * prior   color priorities in descending order
 * ------------------------------------------------------------------
 * bit 0   PL0    PL1    PL2    PL3    PF0    PF1    PF2    PF3/P4 BK
 *         all players in front of all playfield colors
 * bit 1   PL0    PL1    PF0    PF1    PF2    PF3/P4 PL2    PL3    BK
 *         pl 0+1 in front of pf 0-3 in front of pl 2+3
 * bit 2   PF0    PF1    PF2    PF3/P4 PL0    PL1    PL2    PL3    BK
 *         all playfield colors in front of all players
 * bit 3   PF0    PF1    PL0    PL1    PL2    PL3    PF2    PF3/P4 BK
 *         pf 0+1 in front of all players in front of pf 2+3
 * bit 4   missiles colors are PF3 (P4)
 *         missiles have the same priority as pf3
 * bit 5   PL0+PL1 and PL2+PL3 bits xored
 *         00: playfield, 01: PL0/2, 10: PL1/3 11: black (EOR)
 * bit 7+6 CTIA mod (00) or GTIA mode 1 to 3 (01, 10, 11)
 *************************************************************************/

/* player/missile #4 color is equal to playfield #3 */
#define PM4 PF3

/* bit masks for players and missiles */
#define P0 0x01
#define P1 0x02
#define P2 0x04
#define P3 0x08
#define M0 0x10
#define M1 0x20
#define M2 0x40
#define M3 0x80

/************************************************************************
 * Contents of the following table:
 *
 * PL0 -PL3  are the player/missile colors 0 to 3
 * P000-P011 are the 4 available color clocks for playfield color 0
 * P100-P111 are the 4 available color clocks for playfield color 1
 * P200-P211 are the 4 available color clocks for playfield color 2
 * P300-P311 are the 4 available color clocks for playfield color 3
 * ILL       is some undefined color. On my 800XL it looked light yellow ;)
 *
 * Each line holds the 8 bitmasks and resulting colors for player and
 * missile number 0 to 3 in their fixed priority order.
 * The 8 lines per block are for the 8 available playfield colors.
 * Yes, 8 colors because the text modes 2,3 and graphics mode F can
 * be combined with players. The result is the players color with
 * luminance of the modes foreground (ie. colpf1).
 * Any combination of players/missiles (256) is checked for the highest
 * priority player or missile and the resulting color is stored into
 * antic.prio_table. The second part (20-3F) contains the resulting
 * color values for the EOR mode, which is derived from the *visible*
 * player/missile colors calculated for the first part (00-1F).
 * The priorities of combining priority bits (which games use!) are:
 ************************************************************************/
static const UINT8 _pm_colors[32][8*2*8] = {
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 00
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 01
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 02
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2,   0,P2,   0,M3,   0,P3,   0,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2,   0,P2,   0,M3,   0,P3,   0,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 03
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 04
		M0,   0,P0,   0,M1,   0,P1,   0,M2,   0,P2,   0,M3,   0,P3,   0,
		M0,   0,P0,   0,M1,   0,P1,   0,M2,   0,P2,   0,M3,   0,P3,   0,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 05
		M0, ILL,P0, ILL,M1, ILL,P1, ILL,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0,   0,P0,   0,M1,   0,P1,   0,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 06
		M0,   0,P0, ILL,M1,   0,P1, ILL,M2,   0,P2,   0,M3,   0,P3,   0,
		M0,   0,P0,   0,M1,   0,P1,   0,M2,   0,P2,   0,M3,   0,P3,   0,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 07
		M0, ILL,P0, ILL,M1, ILL,P1, ILL,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0,   0,P0,   0,M1,   0,P1,   0,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 08
		M0,   0,P0,   0,M1,   0,P1,   0,M2,   0,P2,   0,M3,   0,P3,   0,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 09
		M0, ILL,P0, ILL,M1, ILL,P1, ILL,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 0A
		M0, ILL,P0, ILL,M1, ILL,P1, ILL,M2,   0,P2,   0,M3,   0,P3,   0,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 0B
		M0, ILL,P0, ILL,M1, ILL,P1, ILL,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 0C
		M0,   0,P0,   0,M1,   0,P1,   0,M2,   0,P2,   0,M3,   0,P3,   0,
		M0,   0,P0,   0,M1,   0,P1,   0,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 0D
		M0,   0,P0,   0,M1,   0,P1,   0,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0,   0,P0,   0,M1,   0,P1,   0,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 0E
		M0,   0,P0,   0,M1,   0,P1,   0,M2,   0,P2,   0,M3,   0,P3,   0,
		M0,   0,P0,   0,M1,   0,P1,   0,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 0F
		M0,   0,P0,   0,M1,   0,P1,   0,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0,   0,P0,   0,M1,   0,P1,   0,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 10
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 11
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2, PL2,P3, PL3,  // 12
		P0, PL0,P1, PL1,M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,   0,P3,   0,
		P0, PL0,P1, PL1,M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,   0,P3,   0,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,M0,P400,M1,P400,M2,P400,M3,P400,P2,P200,P3,P300,
		P0,P001,P1,P101,M0,P401,M1,P401,M2,P401,M3,P401,P2,P201,P3,P301,
		P0,P010,P1,P110,M0,P410,M1,P410,M2,P410,M3,P410,P2,P210,P3,P310,
		P0,P011,P1,P111,M0,P411,M1,P411,M2,P411,M3,P411,P2,P211,P3,P311
	},
	{
		P0, PL0,P1, PL1,M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2, PL2,P3, PL3,  // 13
		P0, PL0,P1, PL1,M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2, PL2,P3, PL3,
		P0, PL0,P1, PL1,M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2, ILL,P3, ILL,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,M0,P400,M1,P400,M2,P400,M3,P400,P2,P200,P3,P300,
		P0,P001,P1,P101,M0,P401,M1,P401,M2,P401,M3,P401,P2,P201,P3,P301,
		P0,P010,P1,P110,M0,P410,M1,P410,M2,P410,M3,P410,P2,P210,P3,P310,
		P0,P011,P1,P111,M0,P411,M1,P411,M2,P411,M3,P411,P2,P211,P3,P311
	},
	{
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P0, PL0,P1, PL1,P2, PL2,P3, PL3,  // 14
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P0,   0,P1,   0,P2,   0,P3,   0,
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P0,   0,P1,   0,P2,   0,P3,   0,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P400,M1,P400,M2,P400,M3,P400,P0,P000,P1,P100,P2,P200,P3,P300,
		M0,P401,M1,P401,M2,P401,M3,P401,P0,P001,P1,P101,P2,P201,P3,P301,
		M0,P410,M1,P410,M2,P410,M3,P410,P0,P010,P1,P110,P2,P210,P3,P310,
		M0,P411,M1,P411,M2,P411,M3,P411,P0,P011,P1,P111,P2,P211,P3,P311
	},
	{
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,P0, PL0,P1, PL1, PL2,P3, PL3,  // 15
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,P0, ILL,P1, ILL, PL2,P3, PL3,
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,P0,	 0,P1,	 0, ILL,P3, ILL,
		 0,   0, 0,   0, 0,   0, 0,   0, 0, 0,	 0, 0,	 0,   0, 0,   0,
		M0,P000,M1,P100,M2,P200,M3,P300,P2,P0,P000,P1,P100,P200,P3,P300,
		M0,P001,M1,P101,M2,P201,M3,P301,P2,P0,P001,P1,P101,P201,P3,P301,
		M0,P010,M1,P110,M2,P210,M3,P310,P2,P0,P010,P1,P110,P210,P3,P310,
		M0,P011,M1,P111,M2,P211,M3,P311,P2,P0,P011,P1,P111,P211,P3,P311
	},
	{
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P0, PL0,P1, PL1,P2, PL2,P3, PL3,  // 16
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P0, ILL,P1, ILL,P2,   0,P3,   0,
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P0,   0,P1,   0,P2,   0,P3,   0,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,M1,P100,M2,P200,M3,P300,P0,P000,P1,P100,P2,P200,P3,P300,
		M0,P001,M1,P101,M2,P201,M3,P301,P0,P001,P1,P101,P2,P201,P3,P301,
		M0,P010,M1,P110,M2,P210,M3,P310,P0,P010,P1,P110,P2,P210,P3,P310,
		M0,P011,M1,P111,M2,P211,M3,P311,P0,P011,P1,P111,P2,P211,P3,P311
	},
	{
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,P0, PL0,P1, PL1, PL2,P3, PL3,  // 17
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,P0, ILL,P1, ILL, PL2,P3, PL3,
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,P0,	 0,P1,	 0, ILL,P3, ILL,
		 0,   0, 0,   0, 0,   0, 0,   0, 0, 0,	 0, 0,	 0,   0, 0,   0,
		M0,P000,M1,P100,M2,P200,M3,P300,P2,P0,P000,P1,P100,P200,P3,P300,
		M0,P001,M1,P101,M2,P201,M3,P301,P2,P0,P001,P1,P101,P201,P3,P301,
		M0,P010,M1,P110,M2,P210,M3,P310,P2,P0,P010,P1,P110,P210,P3,P310,
		M0,P011,M1,P111,M2,P211,M3,P311,P2,P0,P011,P1,P111,P211,P3,P311
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 18
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 19
		P0, ILL,P1, ILL,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P000,M1,P100,M2,P200,M3,P300,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P001,M1,P101,M2,P201,M3,P301,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P010,M1,P110,M2,P210,M3,P310,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P011,M1,P111,M2,P211,M3,P311
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 1A
		P0, ILL,P1, ILL,P2,   0,P3,   0,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0, PL0,P1, PL1,P2, ILL,P3, ILL,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 1B
		P0, ILL,P1, ILL,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0, PL0,P1, PL1,P2, ILL,P3, ILL,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 1C
		P0,   0,P1,   0,P2,   0,P3,   0,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0,   0,P1,   0,P2, ILL,P3, ILL,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 1D
		P0,   0,P1,   0,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0,   0,P1,   0,P2, ILL,P3, ILL,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 1E
		P0,   0,P1,   0,P2,   0,P3,   0,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0,   0,P1,   0,P2, ILL,P3, ILL,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 1F
		P0,   0,P1,   0,P2,   0,P3,   0,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0,   0,P1,   0,P2, ILL,P3, ILL,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	}
};

/************************************************************************
 * prio_init
 * Initialize player/missile priority lookup tables
 ************************************************************************/
static void prio_init(void)
{
	int i, j, pm, p, c;
	const UINT8 * prio;

	/* 32 priority bit combinations */
	for( i = 0; i < 32; i++ )
	{
		/* 8 playfield colors */
		for( j = 0; j < 8; j++ )
		{
			prio = &_pm_colors[i][j*16];
			/* 256 player/missile combinations to build */
			for( pm = 0; pm < 256; pm++ )
			{
				c = PFD; /* assume playfield color */
				for( p = 0; (c == PFD) && (p < 16); p += 2 )
				{
					if (((prio[p] & pm) == prio[p]) && (prio[p+1]))
						c = prio[p+1];
				}
				antic.prio_table[i][(j << 8) + pm] = c;
				if( (c==PL0 || c==P000 || c==P001 || c==P010 || c==P011) &&
					(pm & (P0+P1))==(P0+P1))
					c = EOR;
				if( (c==PL2 || c==P200 || c==P201 || c==P210 || c==P211) &&
					(pm & (P2+P3))==(P2+P3))
					c = EOR;
				antic.prio_table[32 + i][(j << 8) + pm] = c;
			}
		}
	}
}

/************************************************************************
 * cclk_init
 * Initialize "color clock" lookup tables
 ************************************************************************/
static void cclk_init(void)
{
	static const UINT8 _pf_21[4] =   {T00,T01,T10,T11};
	static const UINT8 _pf_1b[4] =   {G00,G01,G10,G11};
	static const UINT8 _pf_210b[4] = {PBK,PF0,PF1,PF2};
	static const UINT8 _pf_310b[4] = {PBK,PF0,PF1,PF3};
	int i;
	UINT8 * dst;

	/* setup color translation for the ANTIC modes */
    for( i = 0; i < 256; i++ )
    {
        /****** text mode (2,3) **********/
		dst = (UINT8 *)&antic.pf_21[0x000+i];
		*dst++ = _pf_21[(i>>6)&3];
		*dst++ = _pf_21[(i>>4)&3];
		*dst++ = _pf_21[(i>>2)&3];
		*dst++ = _pf_21[(i>>0)&3];

        /****** 4 color text (4,5) with pf2, D, E **********/
		dst = (UINT8 *)&antic.pf_x10b[0x000+i];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>0)&3];
		dst = (UINT8 *)&antic.pf_x10b[0x100+i];
		*dst++ = _pf_310b[(i>>6)&3];
		*dst++ = _pf_310b[(i>>4)&3];
		*dst++ = _pf_310b[(i>>2)&3];
		*dst++ = _pf_310b[(i>>0)&3];

        /****** pf0 color text (6,7), 9, B, C **********/
		dst = (UINT8 *)&antic.pf_3210b2[0x000+i*2];
		*dst++ = (i&0x80)?PF0:PBK;
		*dst++ = (i&0x40)?PF0:PBK;
		*dst++ = (i&0x20)?PF0:PBK;
		*dst++ = (i&0x10)?PF0:PBK;
		*dst++ = (i&0x08)?PF0:PBK;
		*dst++ = (i&0x04)?PF0:PBK;
		*dst++ = (i&0x02)?PF0:PBK;
		*dst++ = (i&0x01)?PF0:PBK;

        /****** pf1 color text (6,7), 9, B, C **********/
		dst = (UINT8 *)&antic.pf_3210b2[0x200+i*2];
		*dst++ = (i&0x80)?PF1:PBK;
		*dst++ = (i&0x40)?PF1:PBK;
		*dst++ = (i&0x20)?PF1:PBK;
		*dst++ = (i&0x10)?PF1:PBK;
		*dst++ = (i&0x08)?PF1:PBK;
		*dst++ = (i&0x04)?PF1:PBK;
		*dst++ = (i&0x02)?PF1:PBK;
		*dst++ = (i&0x01)?PF1:PBK;

        /****** pf2 color text (6,7), 9, B, C **********/
		dst = (UINT8 *)&antic.pf_3210b2[0x400+i*2];
		*dst++ = (i&0x80)?PF2:PBK;
		*dst++ = (i&0x40)?PF2:PBK;
		*dst++ = (i&0x20)?PF2:PBK;
		*dst++ = (i&0x10)?PF2:PBK;
		*dst++ = (i&0x08)?PF2:PBK;
		*dst++ = (i&0x04)?PF2:PBK;
		*dst++ = (i&0x02)?PF2:PBK;
		*dst++ = (i&0x01)?PF2:PBK;

        /****** pf3 color text (6,7), 9, B, C **********/
		dst = (UINT8 *)&antic.pf_3210b2[0x600+i*2];
		*dst++ = (i&0x80)?PF3:PBK;
		*dst++ = (i&0x40)?PF3:PBK;
		*dst++ = (i&0x20)?PF3:PBK;
		*dst++ = (i&0x10)?PF3:PBK;
		*dst++ = (i&0x08)?PF3:PBK;
		*dst++ = (i&0x04)?PF3:PBK;
		*dst++ = (i&0x02)?PF3:PBK;
		*dst++ = (i&0x01)?PF3:PBK;

        /****** 4 color graphics 4 cclks (8) **********/
		dst = (UINT8 *)&antic.pf_210b4[i*4];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>0)&3];
		*dst++ = _pf_210b[(i>>0)&3];
		*dst++ = _pf_210b[(i>>0)&3];
		*dst++ = _pf_210b[(i>>0)&3];

        /****** 4 color graphics 2 cclks (A) **********/
		dst = (UINT8 *)&antic.pf_210b2[i*2];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>0)&3];
		*dst++ = _pf_210b[(i>>0)&3];

        /****** high resolution graphics (F) **********/
		dst = (UINT8 *)&antic.pf_1b[i];
		*dst++ = _pf_1b[(i>>6)&3];
		*dst++ = _pf_1b[(i>>4)&3];
		*dst++ = _pf_1b[(i>>2)&3];
		*dst++ = _pf_1b[(i>>0)&3];

        /****** gtia mode 1 **********/
		dst = (UINT8 *)&antic.pf_gtia1[i];
		*dst++ = GT1+((i>>4)&15);
		*dst++ = GT1+((i>>4)&15);
		*dst++ = GT1+(i&15);
		*dst++ = GT1+(i&15);

        /****** gtia mode 2 **********/
		dst = (UINT8 *)&antic.pf_gtia2[i];
		*dst++ = GT2+((i>>4)&15);
		*dst++ = GT2+((i>>4)&15);
		*dst++ = GT2+(i&15);
		*dst++ = GT2+(i&15);

        /****** gtia mode 3 **********/
		dst = (UINT8 *)&antic.pf_gtia3[i];
		*dst++ = GT3+((i>>4)&15);
		*dst++ = GT3+((i>>4)&15);
		*dst++ = GT3+(i&15);
		*dst++ = GT3+(i&15);

    }

	/* setup used color tables */
    for( i = 0; i < 256; i++ )
	{
		/* used colors in text modes 2,3 */
		antic.uc_21[i] = (i) ? PF2 | PF1 : PF2;

		/* used colors in text modes 4,5 and graphics modes D,E */
		switch( i & 0x03 )
		{
			case 0x01: antic.uc_x10b[0x000+i] |= PF0; antic.uc_x10b[0x100+i] |= PF0; break;
			case 0x02: antic.uc_x10b[0x000+i] |= PF1; antic.uc_x10b[0x100+i] |= PF1; break;
			case 0x03: antic.uc_x10b[0x000+i] |= PF2; antic.uc_x10b[0x100+i] |= PF3; break;
		}
		switch( i & 0x0c )
		{
			case 0x04: antic.uc_x10b[0x000+i] |= PF0; antic.uc_x10b[0x100+i] |= PF0; break;
			case 0x08: antic.uc_x10b[0x000+i] |= PF1; antic.uc_x10b[0x100+i] |= PF1; break;
			case 0x0c: antic.uc_x10b[0x000+i] |= PF2; antic.uc_x10b[0x100+i] |= PF3; break;
        }
		switch( i & 0x30 )
		{
			case 0x10: antic.uc_x10b[0x000+i] |= PF0; antic.uc_x10b[0x100+i] |= PF0; break;
			case 0x20: antic.uc_x10b[0x000+i] |= PF1; antic.uc_x10b[0x100+i] |= PF1; break;
			case 0x30: antic.uc_x10b[0x000+i] |= PF2; antic.uc_x10b[0x100+i] |= PF3; break;
		}
		switch( i & 0xc0 )
		{
			case 0x40: antic.uc_x10b[0x000+i] |= PF0; antic.uc_x10b[0x100+i] |= PF0; break;
			case 0x80: antic.uc_x10b[0x000+i] |= PF1; antic.uc_x10b[0x100+i] |= PF1; break;
			case 0xc0: antic.uc_x10b[0x000+i] |= PF2; antic.uc_x10b[0x100+i] |= PF3; break;
        }

		/* used colors in text modes 6,7 and graphics modes 9,B,C */
		if( i )
		{
			antic.uc_3210b2[0x000+i*2] |= PF0;
			antic.uc_3210b2[0x200+i*2] |= PF1;
			antic.uc_3210b2[0x400+i*2] |= PF2;
			antic.uc_3210b2[0x600+i*2] |= PF3;
        }

		/* used colors in graphics mode 8 */
		switch( i & 0x03 )
		{
			case 0x01: antic.uc_210b4[i*4] |= PF0; break;
			case 0x02: antic.uc_210b4[i*4] |= PF1; break;
			case 0x03: antic.uc_210b4[i*4] |= PF2; break;
		}
		switch( i & 0x0c )
		{
			case 0x04: antic.uc_210b4[i*4] |= PF0; break;
			case 0x08: antic.uc_210b4[i*4] |= PF1; break;
			case 0x0c: antic.uc_210b4[i*4] |= PF2; break;
        }
		switch( i & 0x30 )
		{
			case 0x10: antic.uc_210b4[i*4] |= PF0; break;
			case 0x20: antic.uc_210b4[i*4] |= PF1; break;
			case 0x30: antic.uc_210b4[i*4] |= PF2; break;
		}
		switch( i & 0xc0 )
		{
			case 0x40: antic.uc_210b4[i*4] |= PF0; break;
			case 0x80: antic.uc_210b4[i*4] |= PF1; break;
			case 0xc0: antic.uc_210b4[i*4] |= PF2; break;
        }

		/* used colors in graphics mode A */
		switch( i & 0x03 )
		{
			case 0x01: antic.uc_210b2[i*2] |= PF0; break;
			case 0x02: antic.uc_210b2[i*2] |= PF1; break;
			case 0x03: antic.uc_210b2[i*2] |= PF2; break;
		}
		switch( i & 0x0c )
		{
			case 0x04: antic.uc_210b2[i*2] |= PF0; break;
			case 0x08: antic.uc_210b2[i*2] |= PF1; break;
			case 0x0c: antic.uc_210b2[i*2] |= PF2; break;
        }
		switch( i & 0x30 )
		{
			case 0x10: antic.uc_210b2[i*2] |= PF0; break;
			case 0x20: antic.uc_210b2[i*2] |= PF1; break;
			case 0x30: antic.uc_210b2[i*2] |= PF2; break;
		}
		switch( i & 0xc0 )
		{
			case 0x40: antic.uc_210b2[i*2] |= PF0; break;
			case 0x80: antic.uc_210b2[i*2] |= PF1; break;
			case 0xc0: antic.uc_210b2[i*2] |= PF2; break;
        }

		/* used colors in graphics mode F */
		if( i )
			antic.uc_1b[i] |= PF1;

		/* used colors in GTIA graphics modes */
		/* GTIA 1 is 16 different luminances with hue of colbk */
		antic.uc_g1[i] = 0x00;
		/* GTIA 2 is all 9 colors (8..15 is colbk) */
		switch( i & 0x0f )
		{
			case 0x00: antic.uc_g2[i] = 0x10; break;
			case 0x01: antic.uc_g2[i] = 0x20; break;
			case 0x02: antic.uc_g2[i] = 0x40; break;
			case 0x03: antic.uc_g2[i] = 0x80; break;
			case 0x04: antic.uc_g2[i] = 0x01; break;
			case 0x05: antic.uc_g2[i] = 0x02; break;
			case 0x06: antic.uc_g2[i] = 0x04; break;
			case 0x07: antic.uc_g2[i] = 0x08; break;
			default:   antic.uc_g2[i] = 0x00;
        }

        /* GTIA 3 is 16 different hues with luminance of colbk */
		antic.uc_g3[i] = 0x00;
    }
}

/************************************************************************
 * atari_vh_start
 * Initialize the ATARI800 video emulation
 ************************************************************************/
VIDEO_START( atari )
{
	int i;

	LOG(("atari antic_vh_start\n"));
    memset(&antic, 0, sizeof(antic));

	antic.renderer = antic_mode_0_xx;
	antic.cclk_expand = auto_alloc_array(machine, UINT32, 21 * 256);

	antic.pf_21 	  = &antic.cclk_expand[ 0 * 256];
	antic.pf_x10b	  = &antic.cclk_expand[ 1 * 256];
	antic.pf_3210b2   = &antic.cclk_expand[ 3 * 256];
	antic.pf_210b4	  = &antic.cclk_expand[11 * 256];
	antic.pf_210b2	  = &antic.cclk_expand[15 * 256];
	antic.pf_1b 	  = &antic.cclk_expand[17 * 256];
	antic.pf_gtia1	  = &antic.cclk_expand[18 * 256];
	antic.pf_gtia2	  = &antic.cclk_expand[19 * 256];
	antic.pf_gtia3	  = &antic.cclk_expand[20 * 256];

	antic.used_colors = auto_alloc_array(machine, UINT8, 21 * 256);

	memset(antic.used_colors, 0, 21 * 256 * sizeof(UINT8));

	antic.uc_21 	  = &antic.used_colors[ 0 * 256];
	antic.uc_x10b	  = &antic.used_colors[ 1 * 256];
	antic.uc_3210b2   = &antic.used_colors[ 3 * 256];
	antic.uc_210b4	  = &antic.used_colors[11 * 256];
	antic.uc_210b2	  = &antic.used_colors[15 * 256];
	antic.uc_1b 	  = &antic.used_colors[17 * 256];
	antic.uc_g1 	  = &antic.used_colors[18 * 256];
	antic.uc_g2 	  = &antic.used_colors[19 * 256];
	antic.uc_g3 	  = &antic.used_colors[20 * 256];

	/* reset the ANTIC color tables */
	for( i = 0; i < 256; i ++ )
        antic.color_lookup[i] = (machine->pens[0] << 8) + machine->pens[0];

	LOG(("atari cclk_init\n"));
    cclk_init();

	for( i = 0; i < 64; i++ )
    {
		antic.prio_table[i] = auto_alloc_array(machine, UINT8, 8*256);
    }

	LOG(("atari prio_init\n"));
    prio_init();

	for( i = 0; i < machine->primary_screen->height(); i++ )
    {
		antic.video[i] = auto_alloc_clear(machine, VIDEO);
    }

    VIDEO_START_CALL(generic_bitmapped);
}

/************************************************************************
 * atari_vh_screenrefresh
 * Refresh screen bitmap.
 * Note: Actual drawing is done scanline wise during atari_interrupt
 ************************************************************************/
VIDEO_UPDATE( atari )
{
	UINT32 new_tv_artifacts;

	VIDEO_UPDATE_CALL(generic_bitmapped);

	new_tv_artifacts = input_port_read_safe(screen->machine, "artifacts", 0);
	if( tv_artifacts != new_tv_artifacts )
	{
		tv_artifacts = new_tv_artifacts;
	}
	if( atari_frame_counter > 0 )
	{
		if( --atari_frame_counter )
		{
//          ui_draw_text(atari_frame_message, 0, height - 10);
		}
	}
	return 0;
}

static void artifacts_gfx(UINT8 *src, UINT8 *dst, int width)
{
	int x;
	UINT8 n, bits = 0;
	UINT8 b = gtia.w.colbk & 0xf0;
	UINT8 c = gtia.w.colpf1 & 0x0f;
	UINT8 atari_A = ((b+0x30)&0xf0)+c;
	UINT8 atari_B = ((b+0x70)&0xf0)+c;
	UINT8 atari_C = b+c;
	UINT8 atari_D = gtia.w.colbk;

	for( x = 0; x < width * 4; x++ )
	{
		n = *src++;
		bits <<= 2;
		switch( n )
		{
		case G00:
			break;
		case G01:
			bits |= 1;
			break;
		case G10:
			bits |= 2;
			break;
		case G11:
			bits |= 3;
			break;
		default:
			*dst++ = antic.color_lookup[n];
			*dst++ = antic.color_lookup[n];
			continue;
		}
		switch( (bits >> 1) & 7 )
		{
		case 0: /* 0 0 0 */
		case 1: /* 0 0 1 */
		case 4: /* 1 0 0 */
			*dst++ = atari_D;
			break;
		case 3: /* 0 1 1 */
		case 6: /* 1 1 0 */
		case 7: /* 1 1 1 */
			*dst++ = atari_C;
			break;
		case 2: /* 0 1 0 */
			*dst++ = atari_B;
			break;
		case 5: /* 1 0 1 */
			*dst++ = atari_A;
			break;
		}
		switch( bits & 7 )
		{
		case 0: /* 0 0 0 */
		case 1: /* 0 0 1 */
		case 4: /* 1 0 0 */
			*dst++ = atari_D;
			break;
		case 3: /* 0 1 1 */
		case 6: /* 1 1 0 */
		case 7: /* 1 1 1 */
			*dst++ = atari_C;
			break;
		case 2: /* 0 1 0 */
			*dst++ = atari_A;
			break;
		case 5: /* 1 0 1 */
			*dst++ = atari_B;
			break;
        }
    }
}

static void artifacts_txt(UINT8 * src, UINT8 * dst, int width)
{
	int x;
	UINT8 n, bits = 0;
	UINT8 b = gtia.w.colpf2 & 0xf0;
	UINT8 c = gtia.w.colpf1 & 0x0f;
	UINT8 atari_A = ((b+0x30)&0xf0)+c;
	UINT8 atari_B = ((b+0x70)&0xf0)+c;
	UINT8 atari_C = b+c;
	UINT8 atari_D = gtia.w.colpf2;

	for( x = 0; x < width * 4; x++ )
	{
		n = *src++;
		bits <<= 2;
		switch( n )
		{
		case T00:
			break;
		case T01:
			bits |= 1;
			break;
		case T10:
			bits |= 2;
			break;
		case T11:
			bits |= 3;
			break;
		default:
			*dst++ = antic.color_lookup[n];
			*dst++ = antic.color_lookup[n];
			continue;
        }
		switch( (bits >> 1) & 7 )
		{
		case 0: /* 0 0 0 */
		case 1: /* 0 0 1 */
		case 4: /* 1 0 0 */
			*dst++ = atari_D;
			break;
		case 3: /* 0 1 1 */
		case 6: /* 1 1 0 */
		case 7: /* 1 1 1 */
			*dst++ = atari_C;
			break;
		case 2: /* 0 1 0 */
			*dst++ = atari_A;
			break;
		case 5: /* 1 0 1 */
			*dst++ = atari_B;
			break;
		}
		switch( bits & 7 )
		{
		case 0:/* 0 0 0 */
		case 1:/* 0 0 1 */
		case 4:/* 1 0 0 */
			*dst++ = atari_D;
			break;
		case 3: /* 0 1 1 */
		case 6: /* 1 1 0 */
		case 7: /* 1 1 1 */
			*dst++ = atari_C;
			break;
		case 2: /* 0 1 0 */
			*dst++ = atari_B;
			break;
		case 5: /* 1 0 1 */
			*dst++ = atari_A;
			break;
        }
    }
}


static void antic_linerefresh(running_machine *machine)
{
	int x, y;
	UINT8 *src;
	UINT32 *dst;
	UINT32 scanline[4 + (HCHARS * 2) + 4];

	/* increment the scanline */
    if( ++antic.scanline == machine->primary_screen->height() )
    {
        /* and return to the top if the frame was complete */
        antic.scanline = 0;
        antic.modelines = 0;
        /* count frames gone since last write to hitclr */
        gtia.h.hitclr_frames++;
    }

	if( antic.scanline < MIN_Y || antic.scanline > MAX_Y )
        return;

	y = antic.scanline - MIN_Y;
	src = &antic.cclock[PMOFFSET - antic.hscrol_old + 12];
	dst = scanline;

	if( tv_artifacts )
	{
		if( (antic.cmd & 0x0f) == 2 || (antic.cmd & 0x0f) == 3 )
		{
			artifacts_txt(src, (UINT8*)(dst + 3), HCHARS);
			return;
		}
		else
		if( (antic.cmd & 0x0f) == 15 )
		{
			artifacts_gfx(src, (UINT8*)(dst + 3), HCHARS);
			return;
		}
	}
	dst[0] = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
	dst[1] = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
	dst[2] = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
	if ( (antic.cmd & ANTIC_HSCR) == 0  || (antic.pfwidth == 48) || (antic.pfwidth == 32))
	{
		/* no hscroll */
		dst[3] = antic.color_lookup[src[BYTE_XOR_LE(0)]] | antic.color_lookup[src[BYTE_XOR_LE(1)]] << 16;
		src += 2;
		dst += 4;
		for( x = 1; x < HCHARS-1; x++ )
		{
			*dst++ = antic.color_lookup[src[BYTE_XOR_LE(0)]] | antic.color_lookup[src[BYTE_XOR_LE(1)]] << 16;
			*dst++ = antic.color_lookup[src[BYTE_XOR_LE(2)]] | antic.color_lookup[src[BYTE_XOR_LE(3)]] << 16;
			src += 4;
		}
		dst[0] = antic.color_lookup[src[BYTE_XOR_LE(0)]] | antic.color_lookup[src[BYTE_XOR_LE(1)]] << 16;
	}
	else
	{
		/* if hscroll is enabled, more data are fetched by ANTIC, but it still renders playfield
           of width defined by pfwidth. */
		switch( antic.pfwidth )
		{
			case 0:
				{
					dst[3] = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
					dst += 4;
					for ( x = 1; x < HCHARS-1; x++ )
					{
						*dst++ = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
						*dst++ = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
					}
					dst[0] = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
				}
				break;
			/* support for narrow playfield (32) with horizontal scrolling should be added here */
			case 40:
				{
					dst[3] = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
					dst += 4;
					for ( x = 1; x < HCHARS-2; x++ )
					{
						if ( x == 1 )
						{
							*dst++ = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
						}
						else
						{
							*dst++ = antic.color_lookup[src[BYTE_XOR_LE(0)]] | antic.color_lookup[src[BYTE_XOR_LE(1)]] << 16;
						}
						*dst++ = antic.color_lookup[src[BYTE_XOR_LE(2)]] | antic.color_lookup[src[BYTE_XOR_LE(3)]] << 16;
						src += 4;
					}
					for ( ; x < HCHARS-1; x++ )
					{
						*dst++ = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
						*dst++ = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
					}
					dst[0] = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
				}
				break;
		}
	}
	dst[1] = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
	dst[2] = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;
	dst[3] = antic.color_lookup[PBK] | antic.color_lookup[PBK] << 16;

	draw_scanline8(machine->generic.tmpbitmap, 12, y, MIN(machine->generic.tmpbitmap->width - 12, sizeof(scanline)), (const UINT8 *) scanline, NULL);
}

static int cycle(running_machine *machine)
{
	return machine->primary_screen->hpos() * CYCLES_PER_LINE / machine->primary_screen->width();
}

static void after(running_machine *machine, int cycles, timer_fired_func function, const char *funcname)
{
    attotime duration = attotime_make(0, attotime_to_attoseconds(machine->primary_screen->scan_period()) * cycles / CYCLES_PER_LINE);
    (void)funcname;
	LOG(("           after %3d (%5.1f us) %s\n", cycles, attotime_to_double(duration) * 1.0e6, funcname));
	timer_set(machine, duration, NULL, 0, function);
}

static TIMER_CALLBACK( antic_issue_dli )
{
	if( antic.w.nmien & DLI_NMI )
	{
		LOG(("           @cycle #%3d issue DLI\n", cycle(machine)));
		antic.r.nmist |= DLI_NMI;
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE);
	}
	else
	{
		LOG(("           @cycle #%3d DLI not enabled\n", cycle(machine)));
    }
}


static const atari_renderer_func renderer[2][19][5] = {
	/*   no playfield    narrow          normal          wide         */
	{
		{antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx},
		{antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx},
		{antic_mode_0_xx,antic_mode_2_32,antic_mode_2_40,antic_mode_2_48},
		{antic_mode_0_xx,antic_mode_3_32,antic_mode_3_40,antic_mode_3_48},
		{antic_mode_0_xx,antic_mode_4_32,antic_mode_4_40,antic_mode_4_48},
		{antic_mode_0_xx,antic_mode_5_32,antic_mode_5_40,antic_mode_5_48},
		{antic_mode_0_xx,antic_mode_6_32,antic_mode_6_40,antic_mode_6_48},
		{antic_mode_0_xx,antic_mode_7_32,antic_mode_7_40,antic_mode_7_48},
		{antic_mode_0_xx,antic_mode_8_32,antic_mode_8_40,antic_mode_8_48},
		{antic_mode_0_xx,antic_mode_9_32,antic_mode_9_40,antic_mode_9_48},
		{antic_mode_0_xx,antic_mode_a_32,antic_mode_a_40,antic_mode_a_48},
		{antic_mode_0_xx,antic_mode_b_32,antic_mode_b_40,antic_mode_b_48},
		{antic_mode_0_xx,antic_mode_c_32,antic_mode_c_40,antic_mode_c_48},
		{antic_mode_0_xx,antic_mode_d_32,antic_mode_d_40,antic_mode_d_48},
		{antic_mode_0_xx,antic_mode_e_32,antic_mode_e_40,antic_mode_e_48},
		{antic_mode_0_xx,antic_mode_f_32,antic_mode_f_40,antic_mode_f_48},
		{antic_mode_0_xx, gtia_mode_1_32, gtia_mode_1_40, gtia_mode_1_48},
		{antic_mode_0_xx, gtia_mode_2_32, gtia_mode_2_40, gtia_mode_2_48},
		{antic_mode_0_xx, gtia_mode_3_32, gtia_mode_3_40, gtia_mode_3_48},
	},
	/*   with hscrol enabled playfield width is +32 color clocks      */
	/*   no playfield    narrow->normal  normal->wide    wide->wide   */
	{
		{antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx},
		{antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx},
		{antic_mode_0_xx,antic_mode_2_40,antic_mode_2_48,antic_mode_2_48},
		{antic_mode_0_xx,antic_mode_3_40,antic_mode_3_48,antic_mode_3_48},
		{antic_mode_0_xx,antic_mode_4_40,antic_mode_4_48,antic_mode_4_48},
		{antic_mode_0_xx,antic_mode_5_40,antic_mode_5_48,antic_mode_5_48},
		{antic_mode_0_xx,antic_mode_6_40,antic_mode_6_48,antic_mode_6_48},
		{antic_mode_0_xx,antic_mode_7_40,antic_mode_7_48,antic_mode_7_48},
		{antic_mode_0_xx,antic_mode_8_40,antic_mode_8_48,antic_mode_8_48},
		{antic_mode_0_xx,antic_mode_9_40,antic_mode_9_48,antic_mode_9_48},
		{antic_mode_0_xx,antic_mode_a_40,antic_mode_a_48,antic_mode_a_48},
		{antic_mode_0_xx,antic_mode_b_40,antic_mode_b_48,antic_mode_b_48},
		{antic_mode_0_xx,antic_mode_c_40,antic_mode_c_48,antic_mode_c_48},
		{antic_mode_0_xx,antic_mode_d_40,antic_mode_d_48,antic_mode_d_48},
		{antic_mode_0_xx,antic_mode_e_40,antic_mode_e_48,antic_mode_e_48},
		{antic_mode_0_xx,antic_mode_f_40,antic_mode_f_48,antic_mode_f_48},
		{antic_mode_0_xx, gtia_mode_1_40, gtia_mode_1_48, gtia_mode_1_48},
		{antic_mode_0_xx, gtia_mode_2_40, gtia_mode_2_48, gtia_mode_2_48},
		{antic_mode_0_xx, gtia_mode_3_40, gtia_mode_3_48, gtia_mode_3_48},
	}
};

/*****************************************************************************
 *
 *  Antic Line Done
 *
 *****************************************************************************/
static TIMER_CALLBACK( antic_line_done )
{
	LOG(("           @cycle #%3d antic_line_done\n", cycle(machine)));
	if( antic.w.wsync )
    {
		LOG(("           @cycle #%3d release WSYNC\n", cycle(machine)));
        /* release the CPU if it was actually waiting for HSYNC */
        cpuexec_trigger(machine, TRIGGER_HSYNC);
        /* and turn off the 'wait for hsync' flag */
        antic.w.wsync = 0;
    }
	LOG(("           @cycle #%3d release CPU\n", cycle(machine)));
    /* release the CPU (held for emulating cycles stolen by ANTIC DMA) */
	cpuexec_trigger(machine, TRIGGER_STEAL);

	/* refresh the display (translate color clocks to pixels) */
    antic_linerefresh(machine);
}

/*****************************************************************************
 *
 *  Antic Steal Cycles
 *  This is called once per scanline by a interrupt issued in the
 *  atari_scanline_render function. Set a new timer for the HSYNC
 *  position and release the CPU; but hold it again immediately until
 *  TRIGGER_HSYNC if WSYNC (D01A) was accessed
 *
 *****************************************************************************/
static TIMER_CALLBACK( antic_steal_cycles )
{
	LOG(("           @cycle #%3d steal %d cycles\n", cycle(machine), antic.steal_cycles));
	after(machine, antic.steal_cycles, antic_line_done, "antic_line_done");
    antic.steal_cycles = 0;
	cpu_spinuntil_trigger( machine->device("maincpu"), TRIGGER_STEAL );
}


/*****************************************************************************
 *
 *  Antic Scan Line Render
 *  Render the scanline to the scrbitmap buffer.
 *  Also transport player/missile data to the grafp and grafm registers
 *  of the GTIA if enabled (DMA_PLAYER or DMA_MISSILE)
 *
 *****************************************************************************/
static TIMER_CALLBACK( antic_scanline_render )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	VIDEO *video = antic.video[antic.scanline];
	LOG(("           @cycle #%3d render mode $%X lines to go #%d\n", cycle(machine), (antic.cmd & 0x0f), antic.modelines));

    (*antic.renderer)(space, video);

    /* if player/missile graphics is enabled */
    if( antic.scanline < 256 && (antic.w.dmactl & (DMA_PLAYER|DMA_MISSILE)) )
    {
        /* new player/missile graphics data for every scanline ? */
        if( antic.w.dmactl & DMA_PM_DBLLINE )
        {
            /* transport missile data to GTIA ? */
            if( antic.w.dmactl & DMA_MISSILE )
            {
                antic.steal_cycles += 1;
                atari_gtia_w(space, 0x11, RDPMGFXD(space, 3*256));
            }
            /* transport player data to GTIA ? */
            if( antic.w.dmactl & DMA_PLAYER )
            {
                antic.steal_cycles += 4;
                atari_gtia_w(space, 0x0d, RDPMGFXD(space, 4*256));
                atari_gtia_w(space, 0x0e, RDPMGFXD(space, 5*256));
                atari_gtia_w(space, 0x0f, RDPMGFXD(space, 6*256));
                atari_gtia_w(space, 0x10, RDPMGFXD(space, 7*256));
            }
        }
        else
        {
            /* transport missile data to GTIA ? */
            if( antic.w.dmactl & DMA_MISSILE )
            {
				if( (antic.scanline & 1) == 0 ) 	 /* even line ? */
					antic.steal_cycles += 1;
                atari_gtia_w(space, 0x11, RDPMGFXS(space, 3*128));
            }
            /* transport player data to GTIA ? */
            if( antic.w.dmactl & DMA_PLAYER )
            {
				if( (antic.scanline & 1) == 0 ) 	 /* even line ? */
					antic.steal_cycles += 4;
                atari_gtia_w(space, 0x0d, RDPMGFXS(space, 4*128));
                atari_gtia_w(space, 0x0e, RDPMGFXS(space, 5*128));
                atari_gtia_w(space, 0x0f, RDPMGFXS(space, 6*128));
                atari_gtia_w(space, 0x10, RDPMGFXS(space, 7*128));
            }
        }
    }

    gtia_render(video);

    antic.steal_cycles += CYCLES_REFRESH;
	LOG(("           run CPU for %d cycles\n", CYCLES_HSYNC - CYCLES_HSTART - antic.steal_cycles));
	after(machine, CYCLES_HSYNC - CYCLES_HSTART - antic.steal_cycles, antic_steal_cycles, "antic_steal_cycles");
}



INLINE void LMS(running_machine *machine, int new_cmd)
{
    /**************************************************************
     * If the LMS bit (load memory scan) of the current display
     * list command is set, load the video source address from the
     * following two bytes and split it up into video page/offset.
     * Steal two more cycles from the CPU for fetching the address.
     **************************************************************/
    if( new_cmd & ANTIC_LMS )
    {
    	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
		int addr = RDANTIC(space);
        antic.doffs = (antic.doffs + 1) & DOFFS;
        addr += 256 * RDANTIC(space);
        antic.doffs = (antic.doffs + 1) & DOFFS;
        antic.vpage = addr & VPAGE;
        antic.voffs = addr & VOFFS;
		LOG(("           LMS $%04x\n", addr));
        /* steal two more clock cycles from the cpu */
        antic.steal_cycles += 2;
    }
}

/*****************************************************************************
 *
 *  Antic Scan Line DMA
 *  This is called once per scanline from Atari Interrupt
 *  If the ANTIC DMA is active (DMA_ANTIC) and the scanline not inside
 *  the VBL range (VBL_START - TOTAL_LINES or 0 - VBL_END)
 *  check if all mode lines of the previous ANTIC command were done and
 *  if so, read a new command and set up the renderer function
 *
 *****************************************************************************/
static void antic_scanline_dma(running_machine *machine, int param)
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	LOG(("           @cycle #%3d DMA fetch\n", cycle(machine)));
	if (antic.scanline == VBL_END)
		antic.r.nmist &= ~VBL_NMI;
    if( antic.w.dmactl & DMA_ANTIC )
	{
		if( antic.scanline >= VBL_END && antic.scanline < VBL_START )
		{
			if( antic.modelines <= 0 )
			{
				int h = 0, w = antic.w.dmactl & 3;
				UINT8 vscrol_subtract = 0;
				UINT8 new_cmd;

				new_cmd = RDANTIC(space);
				antic.doffs = (antic.doffs + 1) & DOFFS;
				/* steal at one clock cycle from the CPU for fetching the command */
                antic.steal_cycles += 1;
				LOG(("           ANTIC CMD $%02x\n", new_cmd));
				/* command 1 .. 15 ? */
				if (new_cmd & ANTIC_MODE)
				{
					antic.w.chbasl = 0;
					/* vertical scroll mode changed ? */
					if( (antic.cmd ^ new_cmd) & ANTIC_VSCR )
					{
						/* vertical scroll activate now ? */
						if( new_cmd & ANTIC_VSCR )
						{
							antic.vscrol_old =
							vscrol_subtract =
							antic.w.chbasl = antic.w.vscrol;
						}
						else
						{
							vscrol_subtract = ~antic.vscrol_old;
						}
					}
					/* does this command have horizontal scroll enabled ? */
					if( new_cmd & ANTIC_HSCR )
					{
						h = 1;
						antic.hscrol_old = antic.w.hscrol;
					}
					else
					{
						antic.hscrol_old = 0;
					}
				}
				/* Set the ANTIC mode renderer function */
				antic.renderer = renderer[h][new_cmd & ANTIC_MODE][w];

				switch( new_cmd & ANTIC_MODE )
				{
				case 0x00:
					/* generate 1 .. 8 empty lines */
					antic.modelines = ((new_cmd >> 4) & 7) + 1;
					/* did the last ANTIC command have vertical scroll enabled ? */
					if( antic.cmd & ANTIC_VSCR )
					{
						/* yes, generate vscrol_old additional empty lines */
						antic.modelines += antic.vscrol_old;
					}
					/* leave only bit 7 (DLI) set in ANTIC command */
					new_cmd &= ANTIC_DLI;
					break;
				case 0x01:
					/* ANTIC "jump" with DLI: issue interrupt immediately */
					if( new_cmd & ANTIC_DLI )
					{
						/* remove the DLI bit */
						new_cmd &= ~ANTIC_DLI;
						after(machine, CYCLES_DLI_NMI, antic_issue_dli, "antic_issue_dli");
					}
					/* load memory scan bit set ? */
					if( new_cmd & ANTIC_LMS )
					{
						int addr = RDANTIC(space);
                        antic.doffs = (antic.doffs + 1) & DOFFS;
                        addr += 256 * RDANTIC(space);
                        antic.dpage = addr & DPAGE;
                        antic.doffs = addr & DOFFS;
                        /* produce empty scanlines until vblank start */
						antic.modelines = VBL_START + 1 - antic.scanline;
						if( antic.modelines < 0 )
							antic.modelines = machine->primary_screen->height() - antic.scanline;
						LOG(("           JVB $%04x\n", antic.dpage|antic.doffs));
					}
					else
					{
						int addr = RDANTIC(space);
                        antic.doffs = (antic.doffs + 1) & DOFFS;
                        addr += 256 * RDANTIC(space);
                        antic.dpage = addr & DPAGE;
                        antic.doffs = addr & DOFFS;
                        /* produce a single empty scanline */
						antic.modelines = 1;
						LOG(("           JMP $%04x\n", antic.dpage|antic.doffs));
					}
					break;
				case 0x02:
					LMS(machine, new_cmd);
					antic.chbase = (antic.w.chbash & 0xfc) << 8;
					antic.modelines = 8 - (vscrol_subtract & 7);
					if( antic.w.chactl & 4 )	/* decrement chbasl? */
						antic.w.chbasl = antic.modelines - 1;
					break;
				case 0x03:
					LMS(machine, new_cmd);
					antic.chbase = (antic.w.chbash & 0xfc) << 8;
					antic.modelines = 10 - (vscrol_subtract & 9);
					if( antic.w.chactl & 4 )	/* decrement chbasl? */
						antic.w.chbasl = antic.modelines - 1;
					break;
				case 0x04:
					LMS(machine, new_cmd);
					antic.chbase = (antic.w.chbash & 0xfc) << 8;
					antic.modelines = 8 - (vscrol_subtract & 7);
					if( antic.w.chactl & 4 )	/* decrement chbasl? */
						antic.w.chbasl = antic.modelines - 1;
					break;
				case 0x05:
					LMS(machine, new_cmd);
					antic.chbase = (antic.w.chbash & 0xfc) << 8;
					antic.modelines = 16 - (vscrol_subtract & 15);
					if( antic.w.chactl & 4 )	/* decrement chbasl? */
						antic.w.chbasl = antic.modelines - 1;
					break;
				case 0x06:
					LMS(machine, new_cmd);
					antic.chbase = (antic.w.chbash & 0xfe) << 8;
					antic.modelines = 8 - (vscrol_subtract & 7);
					if( antic.w.chactl & 4 )	/* decrement chbasl? */
						antic.w.chbasl = antic.modelines - 1;
					break;
				case 0x07:
					LMS(machine, new_cmd);
					antic.chbase = (antic.w.chbash & 0xfe) << 8;
					antic.modelines = 16 - (vscrol_subtract & 15);
					if( antic.w.chactl & 4 )	/* decrement chbasl? */
						antic.w.chbasl = antic.modelines - 1;
					break;
				case 0x08:
					LMS(machine, new_cmd);
					antic.modelines = 8 - (vscrol_subtract & 7);
					break;
				case 0x09:
					LMS(machine, new_cmd);
					antic.modelines = 4 - (vscrol_subtract & 3);
					break;
				case 0x0a:
					LMS(machine, new_cmd);
					antic.modelines = 4 - (vscrol_subtract & 3);
					break;
				case 0x0b:
					LMS(machine, new_cmd);
					antic.modelines = 2 - (vscrol_subtract & 1);
					break;
				case 0x0c:
					LMS(machine, new_cmd);
					antic.modelines = 1;
                    break;
				case 0x0d:
					LMS(machine, new_cmd);
					antic.modelines = 2 - (vscrol_subtract & 1);
					break;
				case 0x0e:
					LMS(machine, new_cmd);
					antic.modelines = 1;
                    break;
				case 0x0f:
					LMS(machine, new_cmd);
					/* bits 6+7 of the priority select register determine */
					/* if newer GTIA or plain graphics modes are used */
					switch (gtia.w.prior >> 6)
					{
						case 0: break;
						case 1: antic.renderer = renderer[h][16][w];  break;
						case 2: antic.renderer = renderer[h][17][w];  break;
						case 3: antic.renderer = renderer[h][18][w];  break;
					}
					antic.modelines = 1;
                    break;
				}
				/* set new (current) antic command */
				antic.cmd = new_cmd;
            }
        }
		else
		{
			LOG(("           out of visible range\n"));
			antic.cmd = 0x00;
			antic.renderer = antic_mode_0_xx;
        }
	}
	else
	{
		LOG(("           DMA is off\n"));
        antic.cmd = 0x00;
		antic.renderer = antic_mode_0_xx;
	}

	antic.r.nmist &= ~DLI_NMI;
	if( antic.modelines == 1 && (antic.cmd & antic.w.nmien & DLI_NMI) )
		after(machine, CYCLES_DLI_NMI, antic_issue_dli, "antic_issue_dli");

	after(machine, CYCLES_HSTART, antic_scanline_render, "antic_scanline_render");
}

/*****************************************************************************
 *
 *  Generic Atari Interrupt Dispatcher
 *  This is called once per scanline and handles:
 *  vertical blank interrupt
 *  ANTIC DMA to possibly access the next display list command
 *
 *****************************************************************************/

static void generic_atari_interrupt(running_machine *machine, void (*handle_keyboard)(running_machine *machine), int button_count)
{
	int button_port, i;

	LOG(("ANTIC #%3d @cycle #%d scanline interrupt\n", antic.scanline, cycle(machine)));

    if( antic.scanline < VBL_START )
    {
		antic_scanline_dma(machine, 0);
		return;
    }

    if( antic.scanline == VBL_START )
    {
		button_port = input_port_read_safe(machine, "djoy_b", 0);

		/* specify buttons relevant to this Atari variant */
		for (i = 0; i < button_count; i++)
		{
			if ((gtia.w.gractl & GTIA_TRIGGER) == 0)
				gtia.r.but[i] = 1;
			gtia.r.but[i] &= (button_port >> i) & 1;
		}

		/* button registers for xl/xe */
		if (button_count == 2)
		{
			gtia.r.but[2] = 1; /* not used on xl/xe */
			gtia.r.but[3] = 0; /* 1 if external cartridge is inserted */
		}

		handle_keyboard(machine);

		/* do nothing new for the rest of the frame */
		antic.modelines = machine->primary_screen->height() - VBL_START;
		antic.renderer = antic_mode_0_xx;

		/* if the CPU want's to be interrupted at vertical blank... */
		if( antic.w.nmien & VBL_NMI )
		{
			LOG(("           cause VBL NMI\n"));
			/* set the VBL NMI status bit */
			antic.r.nmist |= VBL_NMI;
			cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE);
		}
    }

	/* refresh the display (translate color clocks to pixels) */
    antic_linerefresh(machine);
}



INTERRUPT_GEN( a400_interrupt )
{
	generic_atari_interrupt(device->machine, a800_handle_keyboard, 4);
}

INTERRUPT_GEN( a800_interrupt )
{
	generic_atari_interrupt(device->machine, a800_handle_keyboard, 4);
}

INTERRUPT_GEN( a800xl_interrupt )
{
	generic_atari_interrupt(device->machine, a800_handle_keyboard, 2);
}

INTERRUPT_GEN( a5200_interrupt )
{
	generic_atari_interrupt(device->machine, a5200_handle_keypads, 4);
}
