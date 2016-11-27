/**
2008/06/11, by Naibo(translated to English by Mameplus team):
Driver's Eyes works,
    -the communication work between CPU and 3D DSP should be limited to the master M68000,
    if the address mapping is done in the shared memory, master CPU would be disturbed by the slave one.

    -DIP Switches
    DIP3 ON for Screen on the left
    DIP4 ON for Screen on the right
    should not toggle on both

    -The left, center and right screens have separate programs and boards, each would work independently.
    About projection angles of left and right screen, the angle is correct on "DRIVER'S EYES" title screen, however in the tracks of demo mode it doesn't seem correct.

    -On demo screen, should fog effects be turned off?

    -The game also features a pretty nice 2D sprite layer, which still doesn't show up yet.
    it is known that the CPU does constantly feed the 2D video memory some meaningful and logical data.

Namco System 21

Winning Run
    polygon glitches/flicker
    posirq effects for bitmap layer not working

Driver's Eyes
    not yet working

    Questions? pstroffo@yahoo.com (Phil Stroffolino)

    There are at least four hardware variations, all of which are based on Namco System2:

                          | Winning Run | Driver's Eyes | Starblade | Cyber Sled
--------------------------+-------------+---------------+-----------+------------
GPU+bitmap layer          | yes         | no            | no        | no
Namco System NB1 Sprites  | no          | yes           | yes       | yes
Number of DSPs            | 1           | 1             | 5         | 4

The main 68k CPUs populate a chunk of shared RAM with an display list describing a scene to be rendered.
The main CPUs also specify attributes for a master camera which provides additional global transformations.
The display list contains references to specific 3d objects and their position/orientation in 3d space.

The master DSP parses the display list and applies high level geometry, emitting matrices and object
references.  Object references are expanded into meshes (decoded from point ROMs) and passed to the slave
DSP, interleaved with local transforms.

A collection of slave DSPs transforms, projects, and clips each primitive's vertices.  Each slave DSP outputs
a stream of quad descriptors.

Each quad has a reference color (shared across vertices), and for each vertex the tuple: (screenx,screeny,z-code).
The z-code scalar accounts for depth bias.  A zbuffer is used while rendering quads, and depth cueing is used to
shade pixels according to their depth.

---------------------------------------------------------------------------

STATUS:
    Solvaou:
    Air Combat:
         no polys - why?

    Winning Run
    Winning Run 91
        working
          - some minor polygon glitches
          - posirq handling broken

    Driver's Eyes
        crashes

TODO:   (*) Extract DSP BIOS

TODO:   namcoic.c: in StarBlade, the sprite list is stored at a different location during startup tests.
        What register controls this?

TODO:   Map lamps/vibration outputs as used by StarBlade (and possibly other titles).
        These likely involve the MCU.

---------------------------------------------------------------------------

DSP RAM is shared with the 68000 CPUs and master DSP.
The memory map below reflects DSP RAM as seen by the 68000 CPUs.

   0x200000: ROM:
    0x200010: RAM:
    0x200020: PTR:
    0x200024: <checksum>
    0x200028: <checksum>
    0x200030: SMU: // "NO RESPONS" (DSP)
    0x200040: IDC: // "NO RESPONS" (DSP)
    0x200050: CPU: BOOTING..COMPLETE
    0x200060: DSP:
    0x200070: CRC: OK from cpu
    0x200080: CRC:    from dsp
    0x200090: ID:
    0x2000a0: B-M:
    0x2000b0: P-M:
    0x2000c0: S-M:
   0x200100    status: 2=upload needed, 4=error (abort)
   0x200102    status
   0x200104    0x0002
   0x200106    addr written by main cpu
   0x20010a    point rom checksum (starblade expects 0xed53)
   0x20010c    point rom checksum (starblade expects 0xd5df)
   0x20010e    1 : upload-code-to-dsp request trigger
   0x200110    status
   0x200112    status
   0x200114    master dsp code size
   0x200116    slave dsp code size
   0x200120    upload source1 addr hi
   0x200122    upload source1 addr lo
   0x200124    upload source2 addr hi
   0x200126    upload source2 addr lo
   0x200200    enable
   0x200202    status
   0x200206    work page select
    0x200208   0xa2c2 (air combat)
   0x208000..0x2080ff  camera attributes for page#0
   0x208200..0x208fff  3d object attribute display list for page#0
   0x20c000..0x20c0ff  camera attributes for page#1
   0x20c200..0x20cfff  3d object attribute display list for page#1

       Starblade Cybersled AirCombat22 Solvalou
[400]:= 00 0000   00 0000   00 0000   00 0000
[402]:= 00 0011   00 0011   00 0011   00 0011
[404]:= 00 0000   00 0000    00 0000   00 0000
[406]:= 00 0000   00 0000    00 0000   00 0000
[408]:= 10 1002   10 1000    10 1000   10 1000
[40a]:= 02 0000   00 0000    00 0000   00 ffff
[40c]:= 00 1040   00 023a    00 1000   ff ffff
[40e]:= 00 0000   00 0000    00 0000   ff ffff
[410]:= 10 1034   02 0258    10 1000      ff ffff
[412]:= 40 0000   3a 0000    00 0000   ff ffff
[414]:= 00 1030   00 027a    00 1000      ff ffff
[416]:= 00 0000   00 0000    00 0000      ff ffff
[418]:= 10 1030   02 02a2    10 1000      ff ffff
[41a]:= 34 0000   58 0000    00 0000      ff ffff
[41c]:= 00 1030   00 02c2    00 1000      ff ffff
[41e]:= 00 0000   00 0000    00 0000      ff ffff
[420]:= 10 1030   02 02e6    10 1000      ff ffff

---------------------------------------------------------------------------

Thanks to Aaron Giles for originally making sense of the Point ROM data.

Point data in ROMS (signed 24 bit words) encodes the 3d primitives.

The first part of the Point ROMs is an address table.

Given an object index, this table provides an address into the second part of
the ROM.

The second part of the ROM is a series of display lists.
This is a sequence of pointers to actual polygon data. There may be
more than one, and the list is terminated by $ffffff.

The remainder of the ROM is a series of polygon data. The first word of each
entry is the length of the entry (in words, not counting the length word).

The rest of the data in each entry is organized as follows:

length (1 word)
quad index (1 word) - this increments with each entry
vertex count (1 word) - the number of vertices encoded
unknown value (1 word) - almost always 0; depth bias
vertex list (n x 3 words)
quad count (1 word) - the number of quads to draw
quad primitives (n x 5 words) - color code and four vertex indices

-----------------------------------------------------------------------
Board 1 : DSP Board - 1st PCB. (Uppermost)
DSP Type 1 : 4 x TMS320C25 connected x 4 x Namco Custom chip 67 (68 pin PLCC) (Cybersled)
DSP Type 2 : 5 x TMS320C20 (Starblade)
OSC: 40.000MHz
RAM: HM62832 x 2, M5M5189 x 4, ISSI IS61C68 x 16
ROMS: TMS27C040
Custom Chips:
4 x Namco Custom 327 (24 pin NDIP), each one located next to a chip 67.
4 x Namco Custom chip 342 (160 pin PQFP), there are 3 leds (red/green/yellow) connected to each 342 chip. (12 leds total)
2 x Namco Custom 197 (28 pin NDIP)
Namco Custom chip 317 IDC (180 pin PQFP)
Namco Custom chip 195 (160 pin PQFP)
-----------------------------------------------------------------------
Board 2 : Unknown Board - 2nd PCB (no roms)
OSC: 20.000MHz
RAM: HM62256 x 10, 84256 x 4, CY7C128 x 5, M5M5178 x 4
OTHER Chips:
MB8422-90LP
L7A0565 316 (111) x 1 (100 PIN PQFP)
150 (64 PIN PQFP)
167 (128 PIN PQFP)
L7A0564 x 2 (100 PIN PQFP)
157 x 16 (24 PIN NDIP)
-----------------------------------------------------------------------
Board 3 : CPU Board - 3rd PCB (looks very similar to Namco System 2 CPU PCB)
CPU: MC68000P12 x 2 @ 12 MHz (16-bit)
Sound CPU: MC68B09EP (3 MHz)
Sound Chips: C140 24-channel PCM (Sound Effects), YM2151 (Music), YM3012 (?)
XTAL: 3.579545 MHz
OSC: 49.152 MHz
RAM: MB8464 x 2, MCM2018 x 2, HM65256 x 4, HM62256 x 2

Other Chips:
Sharp PC900 - Opto-isolator
Sharp PC910 - Opto-isolator
HN58C65P (EEPROM)
MB3771
MB87077-SK x 2 (24 pin NDIP, located in sound section)
LB1760 (16 pin DIP, located next to SYS87B-2B)
CY7C132 (48 PIN DIP)

Namco Custom:
148 x 2 (64 pin PQFP)
C68 (64 pin PQFP)
139 (64 pin PQFP)
137 (28 pin NDIP)
149 (28 pin NDIP, near C68)
-----------------------------------------------------------------------
Board 4 : 4th PCB (bottom-most)
OSC: 38.76922 MHz
There is a 6 wire plug joining this PCB with the CPU PCB. It appears to be video cable (RGB, Sync etc..)
Jumpers:
JP7 INTERLACE = SHORTED (Other setting is NON-INTERLACE)
JP8 68000 = SHORTED (Other setting is 68020)
Namco Custom Chips:
C355 (160 pin PQFP)
187 (120 pin PQFP)
138 (64 pin PQFP)
165 (28 pin NDIP)
-----------------------------------------------------------------------

-------------------
Air Combat by NAMCO
-------------------
malcor


Location        Device     File ID      Checksum
-------------------------------------------------
CPU68  1J       27C4001    MPR-L.AC1      9859   [ main program ]  [ rev AC1 ]
CPU68  3J       27C4001    MPR-U.AC1      97F1   [ main program ]  [ rev AC1 ]
CPU68  1J       27C4001    MPR-L.AC2      C778   [ main program ]  [ rev AC2 ]
CPU68  3J       27C4001    MPR-U.AC2      6DD9   [ main program ]  [ rev AC2 ]
CPU68  1C      MB834000    EDATA1-L.AC1   7F77   [    data      ]
CPU68  3C      MB834000    EDATA1-U.AC1   FA2F   [    data      ]
CPU68  3A      MB834000    EDATA-U.AC1    20F2   [    data      ]
CPU68  1A      MB834000    EDATA-L.AC1    9E8A   [    data      ]
CPU68  8J        27C010    SND0.AC1       71A8   [  sound prog  ]
CPU68  12B     MB834000    VOI0.AC1       08CF   [   voice 0    ]
CPU68  12C     MB834000    VOI1.AC1       925D   [   voice 1    ]
CPU68  12D     MB834000    VOI2.AC1       C498   [   voice 2    ]
CPU68  12E     MB834000    VOI3.AC1       DE9F   [   voice 3    ]
CPU68  4C        27C010    SPR-L.AC1      473B   [ slave prog L ]  [ rev AC1 ]
CPU68  6C        27C010    SPR-U.AC1      CA33   [ slave prog U ]  [ rev AC1 ]
CPU68  4C        27C010    SPR-L.AC2      08CE   [ slave prog L ]  [ rev AC2 ]
CPU68  6C        27C010    SPR-U.AC2      A3F1   [ slave prog U ]  [ rev AC2 ]
OBJ(B) 5S       HN62344    OBJ0.AC1       CB72   [ object data  ]
OBJ(B) 5X       HN62344    OBJ1.AC1       85E2   [ object data  ]
OBJ(B) 3S       HN62344    OBJ2.AC1       89DC   [ object data  ]
OBJ(B) 3X       HN62344    OBJ3.AC1       58FF   [ object data  ]
OBJ(B) 4S       HN62344    OBJ4.AC1       46D6   [ object data  ]
OBJ(B) 4X       HN62344    OBJ5.AC1       7B91   [ object data  ]
OBJ(B) 2S       HN62344    OBJ6.AC1       5736   [ object data  ]
OBJ(B) 2X       HN62344    OBJ7.AC1       6D45   [ object data  ]
OBJ(B) 17N     PLHS18P8    3P0BJ3         4342
OBJ(B) 17N     PLHS18P8    3POBJ4         1143
DSP    2N       HN62344    AC1-POIL.L     8AAF   [   DSP data   ]
DSP    2K       HN62344    AC1-POIL.L     CF90   [   DSP data   ]
DSP    2E       HN62344    AC1-POIH       4D02   [   DSP data   ]
DSP    17D     GAL16V8A    3PDSP5         6C00

NOTE:  CPU68  - CPU board        2252961002  (2252971002)
       OBJ(B) - Object board     8623961803  (8623963803)
       DSP    - DSP board        8623961703  (8623963703)
       PGN(C) - PGN board        2252961300  (8623963600)

       Namco System 21 Hardware

       ROMs that have the same locations are different revisions
       of the same ROMs (AC1 or AC2).

Jumper settings:

Location    Position set    alt. setting
----------------------------------------

CPU68 PCB:

  JP2          /D-ST           /VBL
  JP3
*/
#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6805/m6805.h"
#include "deprecat.h"
#include "includes/namcos2.h"
#include "cpu/m6809/m6809.h"
#include "cpu/tms32025/tms32025.h"
#include "includes/namcoic.h"
#include "sound/2151intf.h"
#include "sound/c140.h"
#include "includes/namcos21.h"

#define PTRAM_SIZE 0x20000

static UINT16 *winrun_dspbios;
static UINT16 *winrun_dspcomram;
#define WINRUN_MAX_POLY_PARAM (1+256*3)
static UINT16 winrun_poly_buf[WINRUN_MAX_POLY_PARAM];
static int winrun_poly_index;
static UINT32 winrun_pointrom_addr;
static UINT16 *winrun_polydata;
static int winrun_dsp_alive;
static UINT16 *winrun_gpucomram;
static UINT16 winrun_dspcomram_control[8];
static UINT16 namcos21_video_enable;

static UINT8 *pointram;
static int pointram_idx;

static UINT16 *namcos21_dspram16;
//UINT16 *namcos21_spritepos;

/* private data */
static UINT16 *mpDataROM;
static UINT16 *mpSharedRAM1;
static UINT8	*mpDualPortRAM;
static UINT16 pointram_control;


#define ENABLE_LOGGING		0


#define DSP_BUF_MAX (4096*12)
struct dsp_state
{
	unsigned masterSourceAddr;
	UINT16 slaveInputBuffer[DSP_BUF_MAX];
	unsigned slaveBytesAvailable;
	unsigned slaveBytesAdvertised;
	unsigned slaveInputStart;
	UINT16 slaveOutputBuffer[DSP_BUF_MAX];
	unsigned slaveOutputSize;
	UINT16 masterDirectDrawBuffer[256];
	unsigned masterDirectDrawSize;
	int masterFinished;
	int slaveActive;
};
static dsp_state *mpDspState;

static INT32
ReadPointROMData( running_machine *machine, unsigned offset )
{
	const INT32 *pPointData = (INT32 *)memory_region( machine, "user2" );
	INT32 result = pPointData[offset];
	return result;
}

static READ16_HANDLER(namcos21_video_enable_r)
{
	return namcos21_video_enable;
}

static WRITE16_HANDLER(namcos21_video_enable_w)
{
	COMBINE_DATA( &namcos21_video_enable ); /* 0x40 = enable */
	if( namcos21_video_enable!=0 && namcos21_video_enable!=0x40 )
	{
		logerror( "unexpected namcos21_video_enable_w=0x%x\n", namcos21_video_enable );
	}
}


/***********************************************************/
static WRITE16_HANDLER(dspcuskey_w)
{ /* TODO: proper cuskey emulation */
}

static READ16_HANDLER(dspcuskey_r)
{
	UINT16 result = 0;
	if( namcos2_gametype == NAMCOS21_SOLVALOU )
	{
		switch( cpu_get_pc(space->cpu) )
		{
		case 0x805e: result = 0x0000; break;
		case 0x805f: result = 0xfeba; break;
		case 0x8067: result = 0xffff; break;
		case 0x806e: result = 0x0145; break;
		default:
			logerror( "unk cuskey_r; pc=0x%x\n", cpu_get_pc(space->cpu) );
			break;
		}
	}
	else if( namcos2_gametype == NAMCOS21_CYBERSLED )
	{
		switch( cpu_get_pc(space->cpu) )
		{
		case 0x8061: result = 0xfe95; break;
		case 0x8069: result = 0xffff; break;
		case 0x8070: result = 0x016A; break;
		default:
			break;
		}
	}
	else if( namcos2_gametype == NAMCOS21_AIRCOMBAT )
	{
		switch( cpu_get_pc(space->cpu) )
		{
		case 0x8062: result = 0xfeb9; break;
		case 0x806a: result = 0xffff; break;
		case 0x8071: result = 0x0146; break;
		default:
			break;
		}
	}
	return result;
}

static void
TransmitWordToSlave( UINT16 data )
{
	unsigned offs = mpDspState->slaveInputStart+mpDspState->slaveBytesAvailable++;
	mpDspState->slaveInputBuffer[offs%DSP_BUF_MAX] = data;
	if (ENABLE_LOGGING) logerror( "+%04x(#%04x)\n", data, mpDspState->slaveBytesAvailable );
	mpDspState->slaveActive = 1;
	if( mpDspState->slaveBytesAvailable >= DSP_BUF_MAX )
	{
		logerror( "IDC overflow\n" );
		exit(1);
	}
} /* TransmitWordToSlave */

static void
TransferDspData( running_machine *machine )
{
	UINT16 addr = mpDspState->masterSourceAddr;
	int mode = addr&0x8000;
	addr&=0x7fff;
	if( addr )
	{
		for(;;)
		{
			int i;
			UINT16 old = addr;
			UINT16 code = namcos21_dspram16[addr++];
			if( code == 0xffff )
			{
				if( mode )
				{
					addr = namcos21_dspram16[addr];
					mpDspState->masterSourceAddr = addr;
					if (ENABLE_LOGGING) logerror( "LOOP:0x%04x\n", addr );
					addr&=0x7fff;
					if( old==addr )
					{
						return;
					}
				}
				else
				{
					mpDspState->masterSourceAddr = 0;
					return;
				}
			}
			else if( mode==0 )
			{ /* direct data transfer */
				if (ENABLE_LOGGING) logerror( "DATA TFR(0x%x)\n", code );
				TransmitWordToSlave( code );
				for( i=0; i<code; i++ )
				{
					UINT16 data = namcos21_dspram16[addr++];
					TransmitWordToSlave( data );
				}
			}
			else if( code==0x18 || code==0x1a )
			{
				if (ENABLE_LOGGING) logerror( "HEADER TFR(0x%x)\n", code );
				TransmitWordToSlave( code+1 );
				for( i=0; i<code; i++ )
				{
					UINT16 data = namcos21_dspram16[addr++];
					TransmitWordToSlave( data );
				}
			}
			else
			{
				INT32 masterAddr = ReadPointROMData(machine, code);
				if (ENABLE_LOGGING) logerror( "OBJ TFR(0x%x)\n", code );
				{
					UINT16 len = namcos21_dspram16[addr++];
					for(;;)
					{
						int subAddr = ReadPointROMData(machine, masterAddr++);
						if( subAddr==0xffffff )
						{
							break;
						}
						else
						{
							int primWords = (UINT16)ReadPointROMData(machine, subAddr++);
							if( primWords>2 )
							{
								TransmitWordToSlave( 0 ); /* pad1 */
								TransmitWordToSlave( len+1 );
								for( i=0; i<len; i++ )
								{ /* transform */
									TransmitWordToSlave( namcos21_dspram16[addr+i] );
								}
								TransmitWordToSlave( 0 ); /* pad2 */
								TransmitWordToSlave( primWords+1 );
								for( i=0; i<primWords; i++ )
								{
									TransmitWordToSlave( (UINT16)ReadPointROMData(machine, subAddr+i) );
								}
							}
							else
							{
								if (ENABLE_LOGGING) logerror( "TFR NOP?\n" );
							}
						}
					} /* for(;;) */
					addr+=len;
				}
			}
		} /* for(;;) */
	}
} /* TransferDspData */

static int mbNeedsKickstart;

static UINT16 *master_dsp_code;

void
namcos21_kickstart( running_machine *machine, int internal )
{
	/* patch dsp watchdog */
	switch( namcos2_gametype )
	{
	case NAMCOS21_AIRCOMBAT:
		master_dsp_code[0x008e] = 0x808f;
		break;
	case NAMCOS21_SOLVALOU:
		master_dsp_code[0x008b] = 0x808c;
		break;
	default:
		break;
	}
	if( internal )
	{
		if( mbNeedsKickstart==0 ) return;
		mbNeedsKickstart--;
		if( mbNeedsKickstart ) return;
	}

	namcos21_ClearPolyFrameBuffer();
	mpDspState->masterSourceAddr = 0;
	mpDspState->slaveOutputSize = 0;
	mpDspState->masterFinished = 0;
	mpDspState->slaveActive = 0;
	cputag_set_input_line(machine, "dspmaster", 0, HOLD_LINE);
	cputag_set_input_line(machine, "dspslave", INPUT_LINE_RESET, PULSE_LINE);
}

static UINT16
ReadWordFromSlaveInput( const address_space *space )
{
	UINT16 data = 0;
	if( mpDspState->slaveBytesAvailable>0 )
	{
		data = mpDspState->slaveInputBuffer[mpDspState->slaveInputStart++];
		mpDspState->slaveInputStart %= DSP_BUF_MAX;
		mpDspState->slaveBytesAvailable--;
		if( mpDspState->slaveBytesAdvertised>0 )
		{
			mpDspState->slaveBytesAdvertised--;
		}
		if (ENABLE_LOGGING) logerror( "%s:-%04x(0x%04x)\n", cpuexec_describe_context(space->machine), data, mpDspState->slaveBytesAvailable );
	}
	return data;
} /* ReadWordFromSlaveInput */

static size_t
GetInputBytesAdvertisedForSlave( running_machine *machine )
{
	if( mpDspState->slaveBytesAdvertised < mpDspState->slaveBytesAvailable )
	{
		mpDspState->slaveBytesAdvertised++;
	}
	else if( mpDspState->slaveActive && mpDspState->masterFinished && mpDspState->masterSourceAddr )
	{
		namcos21_kickstart(machine, 0);
	}
	return mpDspState->slaveBytesAdvertised;
} /* GetInputBytesAdvertisedForSlave */

static READ16_HANDLER( dspram16_r )
{
	return namcos21_dspram16[offset];
} /* dspram16_r */

static WRITE16_HANDLER( dspram16_w )
{
	COMBINE_DATA( &namcos21_dspram16[offset] );

	if( namcos2_gametype != NAMCOS21_WINRUN91 )
	{
		if( mpDspState->masterSourceAddr &&
			offset == 1+(mpDspState->masterSourceAddr&0x7fff) )
		{
			if (ENABLE_LOGGING) logerror( "IDC-CONTINUE\n" );
			TransferDspData(space->machine);
		}
		else if (namcos2_gametype == NAMCOS21_SOLVALOU &&
					offset == 0x103 &&
					space->cpu == space->machine->device("maincpu"))
		{ /* hack; synchronization for solvalou */
			cpu_yield(space->cpu);
		}
	}
} /* dspram16_w */

/************************************************************************************/

static int
InitDSP( running_machine *machine )
{
	UINT16 *pMem = (UINT16 *)memory_region(machine, "dspmaster");
	/**
     * DSP BIOS tests "CPU ID" on startup
     * "JAPAN (C)1990 NAMCO LTD. by H.F "
     */
	memcpy( &pMem[0xbff0], &pMem[0x0008], 0x20 );
	pMem[0x8000] = 0xFF80;
	pMem[0x8001] = 0x0000;

	mpDspState = auto_alloc_clear(machine, dsp_state);

	return 0;
}

/***********************************************************/

static UINT32 pointrom_idx;

static UINT8 mPointRomMSB;
static int mbPointRomDataAvailable;

static READ16_HANDLER( dsp_port0_r )
{
	INT32 data = ReadPointROMData(space->machine, pointrom_idx++);
	mPointRomMSB = (UINT8)(data>>16);
	mbPointRomDataAvailable = 1;
	return (UINT16)data;
} /* dsp_port0_r */

static WRITE16_HANDLER( dsp_port0_w )
{ /* unused? */
	if (ENABLE_LOGGING) logerror( "PTRAM_LO(0x%04x)\n", data );
} /* dsp_port0_w */

static READ16_HANDLER( dsp_port1_r )
{
	if( mbPointRomDataAvailable )
	{
		mbPointRomDataAvailable = 0;
		return mPointRomMSB;
	}
	return 0x8000; /* IDC ack? */
} /* dsp_port1_r */

static WRITE16_HANDLER( dsp_port1_w )
{ /* unused? */
	if (ENABLE_LOGGING) logerror( "PTRAM_HI(0x%04x)\n", data );
} /* dsp_port1_w */

static READ16_HANDLER( dsp_port2_r )
{ /* IDC TRANSMIT ENABLE? */
	return 0;
} /* dsp_port2_r */

static WRITE16_HANDLER( dsp_port2_w )
{
	if (ENABLE_LOGGING) logerror( "IDC ADDR INIT(0x%04x)\n", data );
	mpDspState->masterSourceAddr = data;
	TransferDspData(space->machine);
} /* dsp_port2_w */

static READ16_HANDLER( dsp_port3_idc_rcv_enable_r )
{ /* IDC RECEIVE ENABLE? */
	return 0;
} /* dsp_port3_idc_rcv_enable_r */

static WRITE16_HANDLER( dsp_port3_w )
{ /* same address space as pointram? */
	pointrom_idx<<=16;
	pointrom_idx|=data;
} /* dsp_port3_w */

static WRITE16_HANDLER( dsp_port4_w )
{ /* receives $0B<<4 prior to IDC setup */
} /* dsp_port4_w */

static READ16_HANDLER( dsp_port8_r )
{ /* SMU status */
	return 1;
} /* dsp_port8_r */

static int namcos21_irq_enable;

static WRITE16_HANDLER( dsp_port8_w )
{ /* IRQ-enable? */
	if (ENABLE_LOGGING) logerror( "port8_w(%d)\n", data );
	if( data )
	{
		mpDspState->masterFinished = 1;
	}
	namcos21_irq_enable = data;
} /* dsp_port8_w */

static READ16_HANDLER( dsp_port9_r )
{ /* render-device-busy; used for direct-draw */
	return 0;
} /* dsp_port9_r */

static READ16_HANDLER(dsp_porta_r)
{ /* config */
	return 0;
} /* dsp_porta_r */

static WRITE16_HANDLER(dsp_porta_w)
{
	/* boot: 1 */
	/* IRQ0 end: 0 */
	/* INT2 begin: 1 */
	/* direct-draw begin: 0 */
	/* INT1 begin: 1 */
//  if (ENABLE_LOGGING) logerror( "dsp_porta_w(0x%04x)\n", data );
} /* dsp_porta_w */

static READ16_HANDLER(dsp_portb_r)
{ /* config */
	return 1;
} /* dsp_portb_r */

static WRITE16_HANDLER(dsp_portb_w)
{ /* render-device trigger */
	if( data==0 )
	{ /* only 0->1 transition triggers */
		return;
	}
	if( mpDspState->masterDirectDrawSize == 13 )
	{
		int i;
		int sx[4], sy[4], zcode[4];
		int color  = mpDspState->masterDirectDrawBuffer[0];
		for( i=0; i<4; i++ )
		{
			sx[i] = NAMCOS21_POLY_FRAME_WIDTH/2 + (INT16)mpDspState->masterDirectDrawBuffer[i*3+1];
			sy[i] = NAMCOS21_POLY_FRAME_HEIGHT/2 + (INT16)mpDspState->masterDirectDrawBuffer[i*3+2];
			zcode[i] = mpDspState->masterDirectDrawBuffer[i*3+3];
		}
		if( color&0x8000 )
		{
			namcos21_DrawQuad( sx, sy, zcode, color );
		}
		else
		{
			logerror( "indirection used w/ direct draw?\n" );
		}
	}
	else if( mpDspState->masterDirectDrawSize )
	{
		logerror( "unexpected masterDirectDrawSize=%d!\n",mpDspState->masterDirectDrawSize );
	}
	mpDspState->masterDirectDrawSize = 0;
} /* dsp_portb_w */

static WRITE16_HANDLER(dsp_portc_w)
{ /* enqueue data for direct-drawn poly */
	if( mpDspState->masterDirectDrawSize < DSP_BUF_MAX )
	{
		mpDspState->masterDirectDrawBuffer[mpDspState->masterDirectDrawSize++] = data;
	}
	else
	{
		logerror( "portc overflow\n" );
	}
} /* dsp_portc_w */

static READ16_HANDLER( dsp_portf_r )
{ /* informs BIOS that this is Master DSP */
	return 0;
} /* dsp_portf_r */

static WRITE16_HANDLER( dsp_xf_w )
{
	if (ENABLE_LOGGING) logerror("xf(%d)\n",data);
}

static ADDRESS_MAP_START( master_dsp_program, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM /* BIOS */
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_BASE(&master_dsp_code)
ADDRESS_MAP_END

static ADDRESS_MAP_START( master_dsp_data, ADDRESS_SPACE_DATA, 16 )
	AM_RANGE(0x2000, 0x200f) AM_READWRITE(dspcuskey_r,dspcuskey_w)
	AM_RANGE(0x8000, 0xffff) AM_READWRITE(dspram16_r,dspram16_w) /* 0x8000 words */
ADDRESS_MAP_END

static ADDRESS_MAP_START( master_dsp_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x00,0x00) AM_READWRITE(dsp_port0_r,dsp_port0_w)
	AM_RANGE(0x01,0x01) AM_READWRITE(dsp_port1_r,dsp_port1_w)
	AM_RANGE(0x02,0x02) AM_READWRITE(dsp_port2_r,dsp_port2_w)
	AM_RANGE(0x03,0x03) AM_READWRITE(dsp_port3_idc_rcv_enable_r,dsp_port3_w)
	AM_RANGE(0x04,0x04) AM_WRITE(dsp_port4_w)
	AM_RANGE(0x08,0x08) AM_READWRITE(dsp_port8_r,dsp_port8_w)
	AM_RANGE(0x09,0x09) AM_READ(dsp_port9_r)
	AM_RANGE(0x0a,0x0a) AM_READWRITE(dsp_porta_r,dsp_porta_w)
	AM_RANGE(0x0b,0x0b) AM_READWRITE(dsp_portb_r,dsp_portb_w)
	AM_RANGE(0x0c,0x0c) AM_WRITE(dsp_portc_w)
	AM_RANGE(0x0f,0x0f) AM_READ(dsp_portf_r)
	AM_RANGE(TMS32025_HOLD,  TMS32025_HOLD)  AM_READNOP
	AM_RANGE(TMS32025_HOLDA, TMS32025_HOLDA) AM_WRITENOP
	AM_RANGE(TMS32025_XF,    TMS32025_XF)    AM_WRITE( dsp_xf_w )
ADDRESS_MAP_END

/************************************************************************************/

static void
RenderSlaveOutput( UINT16 data )
{
	if( mpDspState->slaveOutputSize >= 4096 )
	{
		logerror( "FATAL ERROR: SLAVE OVERFLOW (0x%x)\n",mpDspState->slaveOutputBuffer[0]  );
		exit(1);
		return;
	}

	/* append word to slave output buffer */
	mpDspState->slaveOutputBuffer[mpDspState->slaveOutputSize++] = data;

	{
		UINT16 *pSource = mpDspState->slaveOutputBuffer;
		UINT16 count = *pSource++;
		if( count && mpDspState->slaveOutputSize > count )
		{
			UINT16 color = *pSource++;
			int sx[4], sy[4],zcode[4];
			int j;
			if( color&0x8000 )
			{
				if( count!=13 ) logerror( "?!direct-draw(%d)\n", count );
				for( j=0; j<4; j++ )
				{
					sx[j] = NAMCOS21_POLY_FRAME_WIDTH/2 + (INT16)pSource[3*j+0];
					sy[j] = NAMCOS21_POLY_FRAME_HEIGHT/2 + (INT16)pSource[3*j+1];
					zcode[j] = pSource[3*j+2];
				}
				namcos21_DrawQuad( sx, sy, zcode, color&0x7fff );
			}
			else
			{
				int quad_idx = color*6;
				for(;;)
				{
					UINT8 code = pointram[quad_idx++];
					color = pointram[quad_idx++]|(code<<8);
					for( j=0; j<4; j++ )
					{
						UINT8 vi = pointram[quad_idx++];
						sx[j] = NAMCOS21_POLY_FRAME_WIDTH/2  + (INT16)pSource[vi*3+0];
						sy[j] = NAMCOS21_POLY_FRAME_HEIGHT/2 + (INT16)pSource[vi*3+1];
						zcode[j] = pSource[vi*3+2];
					}
					namcos21_DrawQuad( sx, sy, zcode, color&0x7fff );
					if( code&0x80 )
					{ /* end-of-quadlist marker */
						break;
					}
				}
			}
			mpDspState->slaveOutputSize = 0;
		}
		else if( count==0 )
		{
			if (ENABLE_LOGGING) logerror( "RenderSlaveOutput\n" );
			exit(1);
		}
	}
} /* RenderSlaveOutput */

static READ16_HANDLER(slave_port0_r)
{
	return ReadWordFromSlaveInput(space);
} /* slave_port0_r */

static WRITE16_HANDLER(slave_port0_w)
{
	RenderSlaveOutput( data );
}

static READ16_HANDLER(slave_port2_r)
{
	return GetInputBytesAdvertisedForSlave(space->machine);
} /* slave_port2_r */

static READ16_HANDLER(slave_port3_r)
{ /* render-device queue size */
	/* up to 0x1fe bytes?
     * slave blocks until free space exists
     */
	return 0;
}

static WRITE16_HANDLER(slave_port3_w)
{ /* 0=busy, 1=ready? */
} /* slave_port3_w */

static WRITE16_HANDLER( slave_XF_output_w )
{
	if (ENABLE_LOGGING) logerror( "0x%x:slaveXF(%d)\n", cpu_get_pc(space->cpu), data );
} /* slave_XF_output_w */

static READ16_HANDLER( slave_portf_r )
{ /* informs BIOS that this is Slave DSP */
	return 1;
}

static ADDRESS_MAP_START( slave_dsp_program, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM /* BIOS */
	AM_RANGE(0x8000, 0x8fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_dsp_data, ADDRESS_SPACE_DATA, 16 )
	/* no external data memory */
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_dsp_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x00,0x00) AM_READWRITE(slave_port0_r,slave_port0_w)
	AM_RANGE(0x02,0x02) AM_READ(slave_port2_r)
	AM_RANGE(0x03,0x03) AM_READWRITE(slave_port3_r,slave_port3_w)
	AM_RANGE(0x0f,0x0f) AM_READ(slave_portf_r)
	AM_RANGE(TMS32025_HOLD,  TMS32025_HOLD)  AM_READNOP
	AM_RANGE(TMS32025_HOLDA, TMS32025_HOLDA) AM_WRITENOP
	AM_RANGE(TMS32025_XF,    TMS32025_XF)    AM_WRITE(slave_XF_output_w)
ADDRESS_MAP_END

/************************************************************************************/

/**
 * 801f->800f : prepare for master access to point ram
 * 801f       : done
 *
 * #bits  data  line
 *   8     1a0    4
 *   7     0f8    4
 *   7     0ff    4
 *   1     001    4
 *   7     00a    2
 *   a     0fe    8
 *
 * line   #bits  data
 * 0003   000A   000004FE
 * 0001   0007   0000000A
 * 0002   001A   03FFF1A0
 */
static WRITE16_HANDLER( pointram_control_w )
{
//  UINT16 prev = pointram_control;
	COMBINE_DATA( &pointram_control );

	/* pointram_control&0x20 : bank for depthcue data */
/*
    logerror( "dsp_control_w:'%s':%x[%x]:=%04x ",
            space->cpu->tag,
            cpu_get_pc(space->cpu),
            offset,
            pointram_control );

    UINT16 delta = (prev^pointram_control)&pointram_control;
    if( delta&0x10 )
    {
        logerror( " [reset]" );
    }
    if( delta&2 )
    {
        logerror( " send(A)%x", pointram_control&1 );
    }
    if( delta&4 )
    {
        logerror( " send(B)%x", pointram_control&1 );
    }
    if( delta&8 )
    {
        logerror( " send(C)%x", pointram_control&1 );
    }
    logerror( "\n" );
*/
	pointram_idx = 0; /* HACK */
} /* pointram_control_w */

static READ16_HANDLER( pointram_data_r )
{
	return pointram[pointram_idx];
} /* pointram_data_r */

static WRITE16_HANDLER( pointram_data_w )
{
	if( ACCESSING_BITS_0_7 )
	{
//      if( (pointram_idx%6)==0 ) logerror("\n" );
//      logerror( " %02x", data );
		pointram[pointram_idx++] = data;
		pointram_idx &= (PTRAM_SIZE-1);
	}
} /* pointram_data_w */

static UINT8 namcos21_depthcue[2][0x400]; /* 0x800 nybbles */

static READ16_HANDLER( namcos21_depthcue_r )
{
	int bank = (pointram_control&0x20)?1:0;
	return namcos21_depthcue[bank][offset];
}
static WRITE16_HANDLER( namcos21_depthcue_w )
{
	if( ACCESSING_BITS_0_7 )
	{
		int bank = (pointram_control&0x20)?1:0;
		namcos21_depthcue[bank][offset] = data;
//      if( (offset&0xf)==0 ) logerror( "\n depthcue: " );
//      logerror( " %02x", data );
	}
}

/* dual port ram memory handlers */

static READ16_HANDLER( namcos2_68k_dualportram_word_r )
{
	return mpDualPortRAM[offset];
}

static WRITE16_HANDLER( namcos2_68k_dualportram_word_w )
{
	if( ACCESSING_BITS_0_7 )
	{
		mpDualPortRAM[offset] = data&0xff;
	}
}

static READ8_HANDLER( namcos2_dualportram_byte_r )
{
	return mpDualPortRAM[offset];
}

static WRITE8_HANDLER( namcos2_dualportram_byte_w )
{
	mpDualPortRAM[offset] = data;
}

/* shared RAM memory handlers */

static READ16_HANDLER( shareram1_r )
{
	return mpSharedRAM1[offset];
}

static WRITE16_HANDLER( shareram1_w )
{
	COMBINE_DATA( &mpSharedRAM1[offset] );
}

/* some games have read-only areas where more ROMs are mapped */

static READ16_HANDLER( datarom_r )
{
	return mpDataROM[offset];
}

static READ16_HANDLER( data2_r )
{
	return mpDataROM[0x100000/2+offset];
}

/* palette memory handlers */

static READ16_HANDLER( paletteram16_r )
{
	return space->machine->generic.paletteram.u16[offset];
}

static WRITE16_HANDLER( paletteram16_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
}

/******************************************************************************/
static WRITE16_HANDLER( NAMCO_C139_SCI_buffer_w ){}
static READ16_HANDLER( NAMCO_C139_SCI_buffer_r ){ return 0; }

static WRITE16_HANDLER( NAMCO_C139_SCI_register_w ){}
static READ16_HANDLER( NAMCO_C139_SCI_register_r ){ return 0; }
/******************************************************************************/

/*************************************************************/
/* MASTER 68000 CPU Memory declarations                      */
/*************************************************************/

#define NAMCO21_68K_COMMON \
	AM_RANGE(0x200000, 0x20ffff) AM_READWRITE(dspram16_r,dspram16_w) AM_BASE(&namcos21_dspram16) \
	AM_RANGE(0x280000, 0x280001) AM_WRITENOP /* written once on startup */ \
	AM_RANGE(0x400000, 0x400001) AM_WRITE(pointram_control_w) \
	AM_RANGE(0x440000, 0x440001) AM_READWRITE(pointram_data_r,pointram_data_w) \
	AM_RANGE(0x440002, 0x47ffff) AM_WRITENOP /* (?) Air Combat */ \
	AM_RANGE(0x480000, 0x4807ff) AM_READWRITE(namcos21_depthcue_r,namcos21_depthcue_w) /* Air Combat */ \
	AM_RANGE(0x700000, 0x71ffff) AM_READWRITE(namco_obj16_r,namco_obj16_w) \
	AM_RANGE(0x720000, 0x720007) AM_READWRITE(namco_spritepos16_r,namco_spritepos16_w) \
	AM_RANGE(0x740000, 0x75ffff) AM_READWRITE(paletteram16_r,paletteram16_w) AM_BASE_GENERIC(paletteram) \
	AM_RANGE(0x760000, 0x760001) AM_READWRITE(namcos21_video_enable_r,namcos21_video_enable_w) \
	AM_RANGE(0x800000, 0x8fffff) AM_READ(datarom_r) \
	AM_RANGE(0x900000, 0x90ffff) AM_READWRITE(shareram1_r,shareram1_w) AM_BASE(&mpSharedRAM1) \
	AM_RANGE(0xa00000, 0xa00fff) AM_READWRITE(namcos2_68k_dualportram_word_r,namcos2_68k_dualportram_word_w) \
	AM_RANGE(0xb00000, 0xb03fff) AM_READWRITE(NAMCO_C139_SCI_buffer_r,NAMCO_C139_SCI_buffer_w) \
	AM_RANGE(0xb80000, 0xb8000f) AM_READWRITE(NAMCO_C139_SCI_register_r,NAMCO_C139_SCI_register_w) \
	AM_RANGE(0xc00000, 0xcfffff) AM_READ(data2_r) /* Cyber Sled */ \
	AM_RANGE(0xd00000, 0xdfffff) AM_READ(data2_r) \

static ADDRESS_MAP_START( namcos21_68k_master, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM /* private work RAM */
	AM_RANGE(0x180000, 0x183fff) AM_READWRITE(NAMCOS2_68K_eeprom_R,NAMCOS2_68K_eeprom_W)// AM_BASE(&namcos2_eeprom) AM_SIZE(&namcos2_eeprom_size)
	AM_RANGE(0x1c0000, 0x1fffff) AM_READWRITE(namcos2_68k_master_C148_r,namcos2_68k_master_C148_w)
	NAMCO21_68K_COMMON
ADDRESS_MAP_END

static ADDRESS_MAP_START( namcos21_68k_slave, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x13ffff) AM_RAM /* private work RAM */
	AM_RANGE(0x1c0000, 0x1fffff) AM_READWRITE(namcos2_68k_slave_C148_r,namcos2_68k_slave_C148_w)
	NAMCO21_68K_COMMON
ADDRESS_MAP_END


/*************************************************************
 * Winning Run is prototype "System21" hardware.
 *************************************************************/
static READ16_HANDLER( winrun_dspcomram_r )
{
	int bank = 1-(winrun_dspcomram_control[0x4/2]&1);
	UINT16 *mem = &winrun_dspcomram[0x1000*bank];
	return mem[offset];
}
static WRITE16_HANDLER( winrun_dspcomram_w )
{
	int bank = 1-(winrun_dspcomram_control[0x4/2]&1);
	UINT16 *mem = &winrun_dspcomram[0x1000*bank];
	COMBINE_DATA( &mem[offset] );
}

static READ16_HANDLER( winrun_cuskey_r )
{
	int pc = cpu_get_pc(space->cpu);
	switch( pc )
	{
	case 0x0064: /* winrun91 */
		return 0xFEBB;
	case 0x006c: /* winrun91 */
		return 0xFFFF;
	case 0x0073: /* winrun91 */
		return 0x0144;

	case 0x0075: /* winrun */
		return 0x24;

	default:
		break;
	}
	return 0;
} /* winrun_cuskey_r */

static WRITE16_HANDLER( winrun_cuskey_w )
{
} /* winrun_cuskey_w */

static void
winrun_flushpoly( void )
{
	if( winrun_poly_index>0 )
	{
		const UINT16 *pSource = winrun_poly_buf;
		UINT16 color;
		int sx[4], sy[4], zcode[4];
		int j;
		color = *pSource++;
		if( color&0x8000 )
		{ /* direct-draw */
			for( j=0; j<4; j++ )
			{
				sx[j] = NAMCOS21_POLY_FRAME_WIDTH/2  + (INT16)*pSource++;
				sy[j] = NAMCOS21_POLY_FRAME_HEIGHT/2 + (INT16)*pSource++;
				zcode[j] = *pSource++;
			}
			namcos21_DrawQuad( sx, sy, zcode, color&0x7fff );
		}
		else
		{
			int quad_idx = color*6;
			for(;;)
			{
				UINT8 code = pointram[quad_idx++];
				color = pointram[quad_idx++];
				for( j=0; j<4; j++ )
				{
					UINT8 vi = pointram[quad_idx++];
					sx[j] = NAMCOS21_POLY_FRAME_WIDTH/2  + (INT16)pSource[vi*3+0];
					sy[j] = NAMCOS21_POLY_FRAME_HEIGHT/2 + (INT16)pSource[vi*3+1];
					zcode[j] = pSource[vi*3+2];
				}
				namcos21_DrawQuad( sx, sy, zcode, color&0x7fff );
				if( code&0x80 )
				{ /* end-of-quadlist marker */
					break;
				}
			}
		}
		winrun_poly_index = 0;
	}
} /* winrun_flushpoly */

static READ16_HANDLER( winrun_poly_reset_r )
{
	winrun_flushpoly();
	return 0;
}

static WRITE16_HANDLER( winrun_dsp_render_w )
{
	if( winrun_poly_index<WINRUN_MAX_POLY_PARAM )
	{
		winrun_poly_buf[winrun_poly_index++] = data;
	}
	else
	{
		logerror( "WINRUN_POLY_OVERFLOW\n" );
	}
} /* winrun_dsp_render_w */

static WRITE16_HANDLER( winrun_dsp_pointrom_addr_w )
{
	if( offset==0 )
	{ /* port 8 */
		winrun_pointrom_addr = data;
	}
	else
	{ /* port 9 */
		winrun_pointrom_addr |= (data<<16);
	}
}

static READ16_HANDLER( winrun_dsp_pointrom_data_r )
{
	UINT16 *ptrom = (UINT16 *)memory_region(space->machine, "user2");
	return ptrom[winrun_pointrom_addr++];
} /* winrun_dsp_pointrom_data_r */

static WRITE16_HANDLER( winrun_dsp_complete_w )
{
	if( data )
	{
		winrun_flushpoly();
		cputag_set_input_line(space->machine, "dsp", INPUT_LINE_RESET, PULSE_LINE);
		namcos21_ClearPolyFrameBuffer();
	}
}

static READ16_HANDLER( winrun_table_r )
{
	return winrun_polydata[offset];
}

static ADDRESS_MAP_START( winrun_dsp_program, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( winrun_dsp_data, ADDRESS_SPACE_DATA, 16 )
	AM_RANGE( 0x2000, 0x200f ) AM_READWRITE(winrun_cuskey_r,winrun_cuskey_w)
	AM_RANGE( 0x4000, 0x4fff ) AM_READWRITE(winrun_dspcomram_r,winrun_dspcomram_w)
	AM_RANGE( 0x8000, 0xffff ) AM_READ(winrun_table_r )
ADDRESS_MAP_END

static ADDRESS_MAP_START( winrun_dsp_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x08,0x09) AM_READWRITE(winrun_dsp_pointrom_data_r,winrun_dsp_pointrom_addr_w)
	AM_RANGE(0x0a,0x0a) AM_WRITE(winrun_dsp_render_w)
	AM_RANGE(0x0b,0x0b) AM_WRITENOP
	AM_RANGE(0x0c,0x0c) AM_WRITE(winrun_dsp_complete_w)
	AM_RANGE(TMS32025_BIO,   TMS32025_BIO)   AM_READ( winrun_poly_reset_r )
	AM_RANGE(TMS32025_HOLD,  TMS32025_HOLD)  AM_READNOP
	AM_RANGE(TMS32025_HOLDA, TMS32025_HOLDA) AM_WRITENOP
	AM_RANGE(TMS32025_XF,    TMS32025_XF)    AM_WRITENOP
ADDRESS_MAP_END

static READ16_HANDLER( gpu_data_r )
{
	const UINT16 *pSrc = (UINT16 *)memory_region( space->machine, "user3" );
	return pSrc[offset];
}

static READ16_HANDLER( winrun_gpucomram_r )
{
	return winrun_gpucomram[offset];
}
static WRITE16_HANDLER( winrun_gpucomram_w )
{
	COMBINE_DATA( &winrun_gpucomram[offset] );
}

static WRITE16_HANDLER( winrun_dspbios_w )
{
	COMBINE_DATA( &winrun_dspbios[offset] );
	if( offset==0xfff )
	{
		UINT16 *mem = (UINT16 *)memory_region(space->machine, "dsp");
		memcpy( mem, winrun_dspbios, 0x2000 );
		winrun_dsp_alive = 1;
	}
} /* winrun_dspbios_w */

//380000 : read : dsp status? 1 = busy
//380000 : write(0x01) - done before dsp comram init
//380004 : dspcomram bank, as seen by 68k
//380008 : read : state?

static READ16_HANDLER( winrun_68k_dspcomram_r )
{
	int bank = winrun_dspcomram_control[0x4/2]&1;
	UINT16 *mem = &winrun_dspcomram[0x1000*bank];
	return mem[offset];
}

static WRITE16_HANDLER( winrun_68k_dspcomram_w )
{
	int bank = winrun_dspcomram_control[0x4/2]&1;
	UINT16 *mem = &winrun_dspcomram[0x1000*bank];
	COMBINE_DATA( &mem[offset] );
}

static READ16_HANDLER( winrun_dspcomram_control_r )
{
	return winrun_dspcomram_control[offset];
}

static WRITE16_HANDLER( winrun_dspcomram_control_w )
{
	COMBINE_DATA( &winrun_dspcomram_control[offset] );
}

static ADDRESS_MAP_START( am_master_winrun, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM /* work RAM */
	AM_RANGE(0x180000, 0x183fff) AM_READWRITE(NAMCOS2_68K_eeprom_R,NAMCOS2_68K_eeprom_W)// AM_BASE(&namcos2_eeprom) AM_SIZE(&namcos2_eeprom_size)
	AM_RANGE(0x1c0000, 0x1fffff) AM_READWRITE(namcos2_68k_master_C148_r,namcos2_68k_master_C148_w)
	AM_RANGE(0x250000, 0x25ffff) AM_RAM AM_BASE( &winrun_polydata )
	AM_RANGE(0x260000, 0x26ffff) AM_RAM /* unused? */
	AM_RANGE(0x280000, 0x281fff) AM_WRITE(winrun_dspbios_w) AM_BASE(&winrun_dspbios)
	AM_RANGE(0x380000, 0x38000f) AM_READWRITE(winrun_dspcomram_control_r,winrun_dspcomram_control_w)
	AM_RANGE(0x3c0000, 0x3c1fff) AM_READWRITE(winrun_68k_dspcomram_r,winrun_68k_dspcomram_w)
	AM_RANGE(0x400000, 0x400001) AM_WRITE(pointram_control_w)
	AM_RANGE(0x440000, 0x440001) AM_READWRITE(pointram_data_r,pointram_data_w)
	AM_RANGE(0x600000, 0x60ffff) AM_READWRITE(winrun_gpucomram_r,winrun_gpucomram_w)
	AM_RANGE(0x800000, 0x87ffff) AM_READ(datarom_r)
	AM_RANGE(0x900000, 0x90ffff) AM_READWRITE(shareram1_r,shareram1_w) AM_BASE(&mpSharedRAM1)
	AM_RANGE(0xa00000, 0xa00fff) AM_READWRITE(namcos2_68k_dualportram_word_r,namcos2_68k_dualportram_word_w)
	AM_RANGE(0xb00000, 0xb03fff) AM_READWRITE(NAMCO_C139_SCI_buffer_r,NAMCO_C139_SCI_buffer_w)
	AM_RANGE(0xb80000, 0xb8000f) AM_READWRITE(NAMCO_C139_SCI_register_r,NAMCO_C139_SCI_register_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( am_slave_winrun, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x13ffff) AM_RAM
	AM_RANGE(0x1c0000, 0x1fffff) AM_READWRITE(namcos2_68k_slave_C148_r,namcos2_68k_slave_C148_w)
	AM_RANGE(0x600000, 0x60ffff) AM_READWRITE(winrun_gpucomram_r,winrun_gpucomram_w)
	AM_RANGE(0x800000, 0x87ffff) AM_READ(datarom_r)
	AM_RANGE(0x900000, 0x90ffff) AM_READWRITE(shareram1_r,shareram1_w)
	AM_RANGE(0xa00000, 0xa00fff) AM_READWRITE(namcos2_68k_dualportram_word_r,namcos2_68k_dualportram_word_w)
	AM_RANGE(0xb00000, 0xb03fff) AM_READWRITE(NAMCO_C139_SCI_buffer_r,NAMCO_C139_SCI_buffer_w)
	AM_RANGE(0xb80000, 0xb8000f) AM_READWRITE(NAMCO_C139_SCI_register_r,NAMCO_C139_SCI_register_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( am_gpu_winrun, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_READWRITE(winrun_gpu_color_r,winrun_gpu_color_w) /* ? */
	AM_RANGE(0x180000, 0x19ffff) AM_RAM /* work RAM */
	AM_RANGE(0x1c0000, 0x1fffff) AM_READWRITE(namcos2_68k_gpu_C148_r,namcos2_68k_gpu_C148_w)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_BASE( &winrun_gpucomram )
	AM_RANGE(0x400000, 0x41ffff) AM_READWRITE(paletteram16_r,paletteram16_w) AM_BASE_GENERIC( paletteram )
	AM_RANGE(0x600000, 0x6fffff) AM_READ(gpu_data_r)
	AM_RANGE(0xc00000, 0xcfffff) AM_READWRITE(winrun_gpu_videoram_r,winrun_gpu_videoram_w)
	AM_RANGE(0xd00000, 0xd0000f) AM_READWRITE(winrun_gpu_register_r,winrun_gpu_register_w)
//  AM_RANGE(0xe0000c, 0xe0000d) POSIRQ
ADDRESS_MAP_END


/*************************************************************/
/* SOUND 6809 CPU Memory declarations                        */
/*************************************************************/

static ADDRESS_MAP_START( am_sound_winrun, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank6") /* banked */
	AM_RANGE(0x3000, 0x3003) AM_WRITENOP /* ? */
	AM_RANGE(0x4000, 0x4001) AM_DEVREADWRITE("ymsnd", ym2151_r,ym2151_w)
	AM_RANGE(0x5000, 0x6fff) AM_DEVREADWRITE("c140", c140_r,c140_w)
	AM_RANGE(0x7000, 0x77ff) AM_READWRITE(namcos2_dualportram_byte_r,namcos2_dualportram_byte_w) AM_BASE(&mpDualPortRAM)
	AM_RANGE(0x7800, 0x7fff) AM_READWRITE(namcos2_dualportram_byte_r,namcos2_dualportram_byte_w) /* mirror */
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xa000, 0xbfff) AM_WRITENOP /* amplifier enable on 1st write */
	AM_RANGE(0xc000, 0xc001) AM_WRITE(namcos2_sound_bankselect_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITENOP /* watchdog */
	AM_RANGE(0xd000, 0xffff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_WRITENOP /* avoid debug log noise; games write frequently to 0xe000 */
ADDRESS_MAP_END


/*************************************************************/
/* I/O HD63705 MCU Memory declarations                       */
/*************************************************************/

static ADDRESS_MAP_START( am_mcu_winrun, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0000) AM_READNOP
	AM_RANGE(0x0001, 0x0001) AM_READ_PORT("PORTB")			/* p1,p2 start */
	AM_RANGE(0x0002, 0x0002) AM_READ_PORT("PORTC")			/* coins */
	AM_RANGE(0x0003, 0x0003) AM_READWRITE(namcos2_mcu_port_d_r,namcos2_mcu_port_d_w)
	AM_RANGE(0x0007, 0x0007) AM_READ_PORT("PORTH")			/* fire buttons */
	AM_RANGE(0x0010, 0x0010) AM_READWRITE(namcos2_mcu_analog_ctrl_r,namcos2_mcu_analog_ctrl_w)
	AM_RANGE(0x0011, 0x0011) AM_READWRITE(namcos2_mcu_analog_port_r,namcos2_mcu_analog_port_w)
	AM_RANGE(0x0000, 0x003f) AM_RAM
	AM_RANGE(0x0040, 0x01bf) AM_RAM
	AM_RANGE(0x01c0, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x2000) AM_READ_PORT("DSW")
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("DIAL0")
	AM_RANGE(0x3001, 0x3001) AM_READ_PORT("DIAL1")
	AM_RANGE(0x3002, 0x3002) AM_READ_PORT("DIAL2")
	AM_RANGE(0x3003, 0x3003) AM_READ_PORT("DIAL3")
	AM_RANGE(0x5000, 0x57ff) AM_READWRITE(namcos2_dualportram_byte_r,namcos2_dualportram_byte_w) AM_BASE(&mpDualPortRAM)
	AM_RANGE(0x6000, 0x6fff) AM_READNOP				/* watchdog */
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define DRIVEYES_68K_COMMON \
	AM_RANGE(0x700000, 0x71ffff) AM_READWRITE(namco_obj16_r,namco_obj16_w) \
	AM_RANGE(0x720000, 0x720007) AM_READWRITE(namco_spritepos16_r,namco_spritepos16_w) \
	AM_RANGE(0x740000, 0x75ffff) AM_READWRITE(paletteram16_r,paletteram16_w) AM_BASE_GENERIC(paletteram) \
	AM_RANGE(0x760000, 0x760001) AM_READWRITE(namcos21_video_enable_r,namcos21_video_enable_w) \
	AM_RANGE(0x800000, 0x8fffff) AM_READ(datarom_r) \
	AM_RANGE(0x900000, 0x90ffff) AM_READWRITE(shareram1_r,shareram1_w) AM_BASE(&mpSharedRAM1) \
	AM_RANGE(0xa00000, 0xa00fff) AM_READWRITE(namcos2_68k_dualportram_word_r,namcos2_68k_dualportram_word_w) \
	AM_RANGE(0xb00000, 0xb03fff) AM_READWRITE(NAMCO_C139_SCI_buffer_r,NAMCO_C139_SCI_buffer_w) \
	AM_RANGE(0xb80000, 0xb8000f) AM_READWRITE(NAMCO_C139_SCI_register_r,NAMCO_C139_SCI_register_w) \


static ADDRESS_MAP_START( driveyes_68k_master, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM /* private work RAM */
	AM_RANGE(0x180000, 0x183fff) AM_READWRITE(NAMCOS2_68K_eeprom_R,NAMCOS2_68K_eeprom_W)// AM_BASE(&namcos2_eeprom) AM_SIZE(&namcos2_eeprom_size)
	AM_RANGE(0x1c0000, 0x1fffff) AM_READWRITE(namcos2_68k_master_C148_r,namcos2_68k_master_C148_w)
	AM_RANGE(0x250000, 0x25ffff) AM_RAM AM_BASE( &winrun_polydata )
	AM_RANGE(0x280000, 0x281fff) AM_WRITE(winrun_dspbios_w) AM_BASE(&winrun_dspbios)
	AM_RANGE(0x380000, 0x38000f) AM_READWRITE(winrun_dspcomram_control_r,winrun_dspcomram_control_w)
	AM_RANGE(0x3c0000, 0x3c1fff) AM_READWRITE(winrun_68k_dspcomram_r,winrun_68k_dspcomram_w)
	AM_RANGE(0x400000, 0x400001) AM_WRITE(pointram_control_w)
	AM_RANGE(0x440000, 0x440001) AM_READWRITE(pointram_data_r,pointram_data_w)
	DRIVEYES_68K_COMMON
ADDRESS_MAP_END

static ADDRESS_MAP_START( driveyes_68k_slave, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM /* private work RAM */
	AM_RANGE(0x1c0000, 0x1fffff) AM_READ(namcos2_68k_slave_C148_r) AM_WRITE(namcos2_68k_slave_C148_w)
	DRIVEYES_68K_COMMON
ADDRESS_MAP_END


static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,4),	/* number of tiles */
	8,		/* bits per pixel */
	{		/* plane offsets */
		0,1,2,3,4,5,6,7
	},
	{ /* x offsets */
		0*8,RGN_FRAC(1,4)+0*8,RGN_FRAC(2,4)+0*8,RGN_FRAC(3,4)+0*8,
		1*8,RGN_FRAC(1,4)+1*8,RGN_FRAC(2,4)+1*8,RGN_FRAC(3,4)+1*8,
		2*8,RGN_FRAC(1,4)+2*8,RGN_FRAC(2,4)+2*8,RGN_FRAC(3,4)+2*8,
		3*8,RGN_FRAC(1,4)+3*8,RGN_FRAC(2,4)+3*8,RGN_FRAC(3,4)+3*8
	},
	{ /* y offsets */
		0*32,1*32,2*32,3*32,
		4*32,5*32,6*32,7*32,
		8*32,9*32,10*32,11*32,
		12*32,13*32,14*32,15*32
	},
	8*64 /* sprite offset */
};

static GFXDECODE_START( namcos21 )
	GFXDECODE_ENTRY( "gfx1", 0x000000, tile_layout,  0x1000, 0x10 )
GFXDECODE_END

static const c140_interface C140_interface_typeA =
{
	C140_TYPE_SYSTEM21_A
};

static const c140_interface C140_interface_typeB =
{
	C140_TYPE_SYSTEM21_B
};

static MACHINE_DRIVER_START( s21base )
	MDRV_CPU_ADD("maincpu", M68000,12288000) /* Master */
	MDRV_CPU_PROGRAM_MAP(namcos21_68k_master)
	MDRV_CPU_VBLANK_INT("screen", namcos2_68k_master_vblank)

	MDRV_CPU_ADD("slave", M68000,12288000) /* Slave */
	MDRV_CPU_PROGRAM_MAP(namcos21_68k_slave)
	MDRV_CPU_VBLANK_INT("screen", namcos2_68k_slave_vblank)

	MDRV_CPU_ADD("audiocpu", M6809,3072000) /* Sound */
	MDRV_CPU_PROGRAM_MAP(am_sound_winrun)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,2)
	MDRV_CPU_PERIODIC_INT(irq1_line_hold,120)

	MDRV_CPU_ADD("mcu", HD63705,2048000) /* IO */
	MDRV_CPU_PROGRAM_MAP(am_mcu_winrun)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("dspmaster", TMS32025,24000000) /* 24 MHz? overclocked */
	MDRV_CPU_PROGRAM_MAP(master_dsp_program)
	MDRV_CPU_DATA_MAP(master_dsp_data)
	MDRV_CPU_IO_MAP(master_dsp_io)

	MDRV_CPU_ADD("dspslave", TMS32025,24000000*4) /* 24 MHz?; overclocked */
	MDRV_CPU_PROGRAM_MAP(slave_dsp_program)
	MDRV_CPU_DATA_MAP(slave_dsp_data)
	MDRV_CPU_IO_MAP(slave_dsp_io)

	MDRV_QUANTUM_TIME(HZ(12000))

	MDRV_MACHINE_START(namcos2)
	MDRV_MACHINE_RESET(namcos2)
	MDRV_NVRAM_HANDLER(namcos2)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(NAMCOS21_POLY_FRAME_WIDTH,NAMCOS21_POLY_FRAME_HEIGHT)
	MDRV_SCREEN_VISIBLE_AREA(0,495,0,479)

	MDRV_GFXDECODE(namcos21)
	MDRV_PALETTE_LENGTH(NAMCOS21_NUM_COLORS)

	MDRV_VIDEO_START(namcos21)
	MDRV_VIDEO_UPDATE(namcos21)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( poly_c140_typeA )
	MDRV_IMPORT_FROM(s21base)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("c140", C140, 8000000/374)
	MDRV_SOUND_CONFIG(C140_interface_typeA)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.50)

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579580)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.30)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.30)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( poly_c140_typeB )
	MDRV_IMPORT_FROM(s21base)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("c140", C140, 8000000/374)
	MDRV_SOUND_CONFIG(C140_interface_typeB)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.50)

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579580)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.30)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.30)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( driveyes )
	MDRV_CPU_ADD("maincpu", M68000,12288000) /* Master */
	MDRV_CPU_PROGRAM_MAP(driveyes_68k_master)
	MDRV_CPU_VBLANK_INT("screen", namcos2_68k_master_vblank)

	MDRV_CPU_ADD("slave", M68000,12288000) /* Slave */
	MDRV_CPU_PROGRAM_MAP(driveyes_68k_slave)
	MDRV_CPU_VBLANK_INT("screen", namcos2_68k_slave_vblank)

	MDRV_CPU_ADD("audiocpu", M6809,3072000) /* Sound */
	MDRV_CPU_PROGRAM_MAP(am_sound_winrun)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,2)
	MDRV_CPU_PERIODIC_INT(irq1_line_hold,120)

	MDRV_CPU_ADD("mcu", HD63705,2048000) /* IO */
	MDRV_CPU_PROGRAM_MAP(am_mcu_winrun)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("dsp", TMS32025,24000000*2) /* 24 MHz? overclocked */
	MDRV_CPU_PROGRAM_MAP(winrun_dsp_program)
	MDRV_CPU_DATA_MAP(winrun_dsp_data)
	MDRV_CPU_IO_MAP(winrun_dsp_io)

	MDRV_QUANTUM_TIME(HZ(6000)) /* 100 CPU slices per frame */

	MDRV_MACHINE_START(namcos2)
	MDRV_MACHINE_RESET(namcos2)
	MDRV_NVRAM_HANDLER(namcos2)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(NAMCOS21_POLY_FRAME_WIDTH,NAMCOS21_POLY_FRAME_HEIGHT)
	MDRV_SCREEN_VISIBLE_AREA(0,495,0,479)

	MDRV_GFXDECODE(namcos21)
	MDRV_PALETTE_LENGTH(NAMCOS21_NUM_COLORS)

	MDRV_VIDEO_START(namcos21)
	MDRV_VIDEO_UPDATE(namcos21)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("c140", C140, 8000000/374)
	MDRV_SOUND_CONFIG(C140_interface_typeA)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.50)

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579580)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.30)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.30)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( winrun_c140_typeB )
	MDRV_CPU_ADD("maincpu", M68000,12288000) /* Master */
	MDRV_CPU_PROGRAM_MAP(am_master_winrun)
	MDRV_CPU_VBLANK_INT("screen", namcos2_68k_master_vblank)

	MDRV_CPU_ADD("slave", M68000,12288000) /* Slave */
	MDRV_CPU_PROGRAM_MAP(am_slave_winrun)
	MDRV_CPU_VBLANK_INT("screen", namcos2_68k_slave_vblank)

	MDRV_CPU_ADD("audiocpu", M6809,3072000) /* Sound */
	MDRV_CPU_PROGRAM_MAP(am_sound_winrun)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,2)
	MDRV_CPU_PERIODIC_INT(irq1_line_hold,120)

	MDRV_CPU_ADD("mcu", HD63705,2048000) /* IO */
	MDRV_CPU_PROGRAM_MAP(am_mcu_winrun)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("dsp", TMS32025,24000000) /* 24 MHz? overclocked */
	MDRV_CPU_PROGRAM_MAP(winrun_dsp_program)
	MDRV_CPU_DATA_MAP(winrun_dsp_data)
	MDRV_CPU_IO_MAP(winrun_dsp_io)

	MDRV_CPU_ADD("gpu", M68000,12288000) /* graphics coprocessor */
	MDRV_CPU_PROGRAM_MAP(am_gpu_winrun)
	MDRV_CPU_VBLANK_INT("screen", namcos2_68k_gpu_vblank)

	MDRV_QUANTUM_TIME(HZ(6000)) /* 100 CPU slices per frame */

	MDRV_MACHINE_START(namcos2)
	MDRV_MACHINE_RESET(namcos2)
	MDRV_NVRAM_HANDLER(namcos2)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(NAMCOS21_POLY_FRAME_WIDTH,NAMCOS21_POLY_FRAME_HEIGHT)
	MDRV_SCREEN_VISIBLE_AREA(0,495,0,479)

	MDRV_PALETTE_LENGTH(NAMCOS21_NUM_COLORS)

	MDRV_VIDEO_START(namcos21)
	MDRV_VIDEO_UPDATE(namcos21)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("c140", C140, 8000000/374)
	MDRV_SOUND_CONFIG(C140_interface_typeB)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.50)

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579580)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.30)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.30)
MACHINE_DRIVER_END

ROM_START( aircomb )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "ac2-mpr-u.3j",  0x000000, 0x80000, CRC(a7133f85) SHA1(9f1c99dd503f1fc81096170fd272e33ae8a7de2f) )
	ROM_LOAD16_BYTE( "ac2-mpr-l.1j",  0x000001, 0x80000, CRC(520a52e6) SHA1(74306e02abfe08aa1afbf325b74dbc0840c3ad3a) )

	ROM_REGION( 0x80000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "ac2-spr-u.6c",  0x000000, 0x20000, CRC(42aca956) SHA1(10ea2400bb4d5b2d805e2de43ca0e0f54597f660) )
	ROM_LOAD16_BYTE( "ac2-spr-l.4c",  0x000001, 0x20000, CRC(3e15fa19) SHA1(65dbb33ab6b3c06c793613348ebb7b110b8bba0d) )

	ROM_REGION( 0x030000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "ac1-snd0.8j", 0x00c000, 0x004000, CRC(5c1fb84b) SHA1(20e4d81289dbe58ffcfc947251a6ff1cc1e36436) )
	ROM_CONTINUE(            0x010000, 0x01c000 )
	ROM_RELOAD(              0x010000, 0x020000 )

	ROM_REGION( 0x010000, "mcu", 0 ) /* I/O MCU */
	ROM_LOAD( "sys2mcpu.bin",  0x000000, 0x002000, CRC(a342a97e) SHA1(2c420d34dba21e409bf78ddca710fc7de65a6642) )
	ROM_LOAD( "sys2c65c.bin",  0x008000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x20000, "dspmaster", 0 ) /* Master DSP */
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )
	ROM_REGION( 0x20000, "dspslave", 0 ) /* Slave DSP */
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "ac2-obj0.5s",  0x000000, 0x80000, CRC(8327ff22) SHA1(16f6022dedb7a74590898bc8ed3e8a97993c4635) )
	ROM_LOAD( "ac2-obj4.4s",  0x080000, 0x80000, CRC(e433e344) SHA1(98ade550cf066fcb5c09fa905f441a1464d4d625) )
	ROM_LOAD( "ac2-obj1.5x",  0x100000, 0x80000, CRC(43af566d) SHA1(99f0d9f005e28040f5cc10de2198893946a31d09) )
	ROM_LOAD( "ac2-obj5.4x",  0x180000, 0x80000, CRC(ecb19199) SHA1(8e0aa1bc1141c4b09576ab08970d0c7629560643) )
	ROM_LOAD( "ac2-obj2.3s",  0x200000, 0x80000, CRC(dafbf489) SHA1(c53ccb3e1b4a6a660bd28c8abe52ccc3f85d111f) )
	ROM_LOAD( "ac2-obj6.2s",  0x280000, 0x80000, CRC(24cc3f36) SHA1(e50af176eb3034c9cab7613ca614f5cc2c62f95e) )
	ROM_LOAD( "ac2-obj3.3x",  0x300000, 0x80000, CRC(bd555a1d) SHA1(96e432b30da6f5f7ccb768c516b1f7186bc0d4c9) )
	ROM_LOAD( "ac2-obj7.2x",  0x380000, 0x80000, CRC(d561fbe3) SHA1(a23976e10bddf74d4a6b292f044dfd0affbab101) )

	ROM_REGION16_BE( 0x200000, "user1", 0 ) /* collision */
	ROM_LOAD16_BYTE( "ac1-data-u.3a",    0x000000, 0x80000, CRC(82320c71) SHA1(2be98d46853febb46e1cc728af2735c0e00ce303) )
	ROM_LOAD16_BYTE( "ac1-data-l.1a",    0x000001, 0x80000, CRC(fd7947d3) SHA1(2696eeae37de6d256e626cc3f3cea7b0f6eff60e) )
	ROM_LOAD16_BYTE( "ac2-edata1-u.3c",  0x100000, 0x80000, CRC(40c07095) SHA1(5d9beaf5bc411ac66785d70980977b08446f46e3) )
	ROM_LOAD16_BYTE( "ac1-edata1-l.1c",  0x100001, 0x80000, CRC(a87087dd) SHA1(cd9b83a8f07886ab44e4ded68002b44338777e8c) )

	ROM_REGION32_BE( 0x400000, "user2", ROMREGION_ERASE )		/* 24bit signed point data */
	ROM_LOAD32_BYTE( "ac1-poi-h.2f",  0x000001, 0x80000, CRC(573bbc3b) SHA1(371be12b915db6872049f18980c1b55544cfc445) )	/* most significant */
	ROM_LOAD32_BYTE( "ac1-poi-lu.2k", 0x000002, 0x80000, CRC(d99084b9) SHA1(c604d60a2162af7610e5ff7c1aa4195f7df82efe) )
	ROM_LOAD32_BYTE( "ac1-poi-ll.2n", 0x000003, 0x80000, CRC(abb32307) SHA1(8e936ba99479215dd33a951d81ec2b04020dfd62) )	/* least significant */

	ROM_REGION( 0x200000, "c140", 0 ) /* sound samples */
	ROM_LOAD("ac1-voi0.12b",0x000000,0x80000,CRC(f427b119) SHA1(bd45bbe41c8be26d6c997fcdc226d080b416a2cf) )
	ROM_LOAD("ac1-voi1.12c",0x080000,0x80000,CRC(c9490667) SHA1(4b6fbe635c32469870a8e6f82742be6a9d4918c9) )
	ROM_LOAD("ac1-voi2.12d",0x100000,0x80000,CRC(1fcb51ba) SHA1(80fc815e5fad76d20c3795ab1d89b57d9abc3efd) )
	ROM_LOAD("ac1-voi3.12e",0x180000,0x80000,CRC(cd202e06) SHA1(72a18f5ba402caefef14b8d1304f337eaaa3eb1d) )

//  ROM_REGION( 0x0600, "plds", 0 )
//  ROM_LOAD( "gal16v8a-3pdsp5.17d", 0x0000, 0x0117, CRC(799c1f26) SHA1(d28ed1b9fa78180c5a0b01a7198a2870137c7349) )
//  ROM_LOAD( "plhs18p8-3pobj3.17n", 0x0200, 0x0149, CRC(9625f469) SHA1(29158a3d37485fb0714d0a60bcd07abd26a3f56e) )
//  ROM_LOAD( "plhs18p8-3pobj4.17n", 0x0400, 0x0149, CRC(1b7c90c1) SHA1(ae65aab7a191cdf1af488e144af22b9d8669c903) )
ROM_END

ROM_START( aircombj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "ac1-mpr-u.3j",  0x000000, 0x80000, CRC(a4dec813) SHA1(2ee8b3492d30db4c841f695151880925a5e205e0) )
	ROM_LOAD16_BYTE( "ac1-mpr-l.1j",  0x000001, 0x80000, CRC(8577b6a2) SHA1(32194e392fbd051754be88eb8c90688c65c65d85) )

	ROM_REGION( 0x080000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "ac1-spr-u.6c",  0x000000, 0x20000, CRC(5810e219) SHA1(c312ffd8324670897871b12d521779570dc0f580) )
	ROM_LOAD16_BYTE( "ac1-spr-l.4c",  0x000001, 0x20000, CRC(175a7d6c) SHA1(9e31dde6646cd9b6dcdbdb3f2326177508559e56) )

	ROM_REGION( 0x030000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "ac1-snd0.8j", 0x00c000, 0x004000, CRC(5c1fb84b) SHA1(20e4d81289dbe58ffcfc947251a6ff1cc1e36436) )
	ROM_CONTINUE(            0x010000, 0x01c000 )
	ROM_RELOAD(              0x010000, 0x020000 )

	ROM_REGION( 0x010000, "mcu", 0 ) /* I/O MCU */
	ROM_LOAD( "sys2mcpu.bin",  0x000000, 0x002000, CRC(a342a97e) SHA1(2c420d34dba21e409bf78ddca710fc7de65a6642) )
	ROM_LOAD( "sys2c65c.bin",  0x008000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x20000, "dspmaster", 0 ) /* Master DSP */
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )
	ROM_REGION( 0x20000, "dspslave", 0 ) /* Slave DSP */
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "ac1-obj0.5s",  0x000000, 0x80000, CRC(d2310c6a) SHA1(9bb8fdfc2c232574777248f4959975f9a20e3105) )
	ROM_LOAD( "ac1-obj4.4s",  0x080000, 0x80000, CRC(0c93b478) SHA1(a92ffbcf04b64e0eee5bcf37008e247700641b25) )
	ROM_LOAD( "ac1-obj1.5x",  0x100000, 0x80000, CRC(f5783a77) SHA1(0be1815ceb4ce4fa7ab75ba588e090f20ee0cac9) )
	ROM_LOAD( "ac1-obj5.4x",  0x180000, 0x80000, CRC(476aed15) SHA1(0e53fdf02e8ffe7852a1fa8bd2f64d0e58f3dc09) )
	ROM_LOAD( "ac1-obj2.3s",  0x200000, 0x80000, CRC(01343d5c) SHA1(64171fed1d1f8682b3d70d3233ea017719f4cc63) )
	ROM_LOAD( "ac1-obj6.2s",  0x280000, 0x80000, CRC(c67607b1) SHA1(df64ea7920cf64271fe742d3d0a57f842ee61e8d) )
	ROM_LOAD( "ac1-obj3.3x",  0x300000, 0x80000, CRC(7717f52e) SHA1(be1df3f4d0fdcaa5d3c81a724e5eb9d14136c6f5) )
	ROM_LOAD( "ac1-obj7.2x",  0x380000, 0x80000, CRC(cfa9fe5f) SHA1(0da25663b89d653c87ed32d15f7c82f3035702ab) )

	ROM_REGION16_BE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "ac1-data-u.3a",   0x000000, 0x80000, CRC(82320c71) SHA1(2be98d46853febb46e1cc728af2735c0e00ce303) )
	ROM_LOAD16_BYTE( "ac1-data-l.1a",   0x000001, 0x80000, CRC(fd7947d3) SHA1(2696eeae37de6d256e626cc3f3cea7b0f6eff60e) )
	ROM_LOAD16_BYTE( "ac1-edata1-u.3c", 0x100000, 0x80000, CRC(a9547509) SHA1(1bc663cec03b60ad968896bbc2546f02efda135e) )
	ROM_LOAD16_BYTE( "ac1-edata1-l.1c", 0x100001, 0x80000, CRC(a87087dd) SHA1(cd9b83a8f07886ab44e4ded68002b44338777e8c) )

	ROM_REGION32_BE( 0x400000, "user2", ROMREGION_ERASE )		/* 24bit signed point data */
	ROM_LOAD32_BYTE( "ac1-poi-h.2f",  0x000001, 0x80000, CRC(573bbc3b) SHA1(371be12b915db6872049f18980c1b55544cfc445) )	/* most significant */
	ROM_LOAD32_BYTE( "ac1-poi-lu.2k", 0x000002, 0x80000, CRC(d99084b9) SHA1(c604d60a2162af7610e5ff7c1aa4195f7df82efe) )
	ROM_LOAD32_BYTE( "ac1-poi-ll.2n", 0x000003, 0x80000, CRC(abb32307) SHA1(8e936ba99479215dd33a951d81ec2b04020dfd62) )	/* least significant */

	ROM_REGION( 0x200000, "c140", 0 ) /* sound samples */
	ROM_LOAD("ac1-voi0.12b",0x000000,0x80000,CRC(f427b119) SHA1(bd45bbe41c8be26d6c997fcdc226d080b416a2cf) )
	ROM_LOAD("ac1-voi1.12c",0x080000,0x80000,CRC(c9490667) SHA1(4b6fbe635c32469870a8e6f82742be6a9d4918c9) )
	ROM_LOAD("ac1-voi2.12d",0x100000,0x80000,CRC(1fcb51ba) SHA1(80fc815e5fad76d20c3795ab1d89b57d9abc3efd) )
	ROM_LOAD("ac1-voi3.12e",0x180000,0x80000,CRC(cd202e06) SHA1(72a18f5ba402caefef14b8d1304f337eaaa3eb1d) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "gal16v8a-3pdsp5.17d", 0x0000, 0x0117, CRC(799c1f26) SHA1(d28ed1b9fa78180c5a0b01a7198a2870137c7349) )
	ROM_LOAD( "plhs18p8-3pobj3.17n", 0x0200, 0x0149, CRC(9625f469) SHA1(29158a3d37485fb0714d0a60bcd07abd26a3f56e) )
	ROM_LOAD( "plhs18p8-3pobj4.17n", 0x0400, 0x0149, CRC(1b7c90c1) SHA1(ae65aab7a191cdf1af488e144af22b9d8669c903) )
ROM_END

ROM_START( cybsled )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "cy2-mpr-u.3j",  0x000000, 0x80000, CRC(b35a72bc) SHA1(d9bc5b8f0bc30510fca8fc57eeb67e5ca0e4c67f) )
	ROM_LOAD16_BYTE( "cy2-mpr-l.1j",  0x000001, 0x80000, CRC(c4a25919) SHA1(52f6947102001376e37730ace16283141b13fee7) )

	ROM_REGION( 0x100000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "cy2-spr-u.6c",  0x000000, 0x80000, CRC(575a422d) SHA1(cad97742da1e2baf47ac110fadef5544b3a30cc7) )
	ROM_LOAD16_BYTE( "cy2-spr-l.4c",  0x000001, 0x80000, CRC(4066291a) SHA1(6ebbc11a68f66ec1e6d2e6ee857e8c599691e289) )

	ROM_REGION( 0x030000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "cy1-snd0.8j", 0x00c000, 0x004000, CRC(3dddf83b) SHA1(e16119cbef176b6f8f8ace773fcbc201e987823f) )
	ROM_CONTINUE(            0x010000, 0x01c000 )
	ROM_RELOAD(              0x010000, 0x020000 )

	ROM_REGION( 0x010000, "mcu", 0 ) /* I/O MCU */
	ROM_LOAD( "sys2mcpu.bin",  0x000000, 0x002000, CRC(a342a97e) SHA1(2c420d34dba21e409bf78ddca710fc7de65a6642) )
	ROM_LOAD( "sys2c65c.bin",  0x008000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x20000, "dspmaster", 0 ) /* Master DSP */
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )
	ROM_REGION( 0x20000, "dspslave", 0 ) /* Slave DSP */
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "cy1-obj0.5s",  0x000000, 0x80000, CRC(5ae542d5) SHA1(99b1a3ed476da4a97cb864538909d7b831f0fd3b) )
	ROM_LOAD( "cy1-obj4.4s",  0x080000, 0x80000, CRC(57904076) SHA1(b1dc0d99543bc4b9584b37ffc12c6ebc59e30e3b) )
	ROM_LOAD( "cy1-obj1.5x",  0x100000, 0x80000, CRC(4aae3eff) SHA1(c80240bd2f4228a0261a14adb6b10560b31b5aa0) )
	ROM_LOAD( "cy1-obj5.4x",  0x180000, 0x80000, CRC(0e11ca47) SHA1(076a9a4cfddbee2d8aaa06110333090d8fdbefeb) )
	ROM_LOAD( "cy1-obj2.3s",  0x200000, 0x80000, CRC(d64ec4c3) SHA1(0bed1cafc21ed8cef3850fb81e30076977086eb0) )
	ROM_LOAD( "cy1-obj6.2s",  0x280000, 0x80000, CRC(7748b485) SHA1(adb4da419a6cdbefd0fef182d866a3479be379af) )
	ROM_LOAD( "cy1-obj3.3x",  0x300000, 0x80000, CRC(3d1f7168) SHA1(392dddcc79fe61dcc6514a91ac27b5e36825d8b7) )
	ROM_LOAD( "cy1-obj7.2x",  0x380000, 0x80000, CRC(b6eb6ad2) SHA1(85a660c5e44012491be7d4e783cce6ba12c135cb) )

	ROM_REGION16_BE( 0x200000, "user1", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "cy1-data-u.3a",   0x000000, 0x80000, CRC(570da15d) SHA1(9ebe756f10756c079a92fb522332e9e52ff715c3) )
	ROM_LOAD16_BYTE( "cy1-data-l.1a",   0x000001, 0x80000, CRC(9cf96f9e) SHA1(91783f48b93e03c778c6641ca8fb419c13b0d3c5) )
	ROM_LOAD16_BYTE( "cy1-edata0-u.3b", 0x100000, 0x80000, CRC(77452533) SHA1(48fc199bcc1beb23c714eebd9b09b153c980170b) )
	ROM_LOAD16_BYTE( "cy1-edata0-l.1b", 0x100001, 0x80000, CRC(e812e290) SHA1(719e0a026ae8ef63d0d0269b67669ea9b4d950dd) )

	ROM_REGION32_BE( 0x400000, "user2", ROMREGION_ERASE )		/* 24bit signed point data */
	ROM_LOAD32_BYTE( "cy1-poi-h1.2f",  0x000001, 0x80000, CRC(eaf8bac3) SHA1(7a2caf6672af158b4a23ce4626342d1f17d1a4e4) )	/* most significant */
	ROM_LOAD32_BYTE( "cy1-poi-lu1.2k", 0x000002, 0x80000, CRC(c544a8dc) SHA1(4cce5f2ab3519b4aa7edbdd15b2d79a7fdcade3c) )
	ROM_LOAD32_BYTE( "cy1-poi-ll1.2n", 0x000003, 0x80000, CRC(30acb99b) SHA1(a28dcb3e5405f166644f6353a903c1143ee268f1) )	/* least significant */
	ROM_LOAD32_BYTE( "cy1-poi-h2.2j",  0x200001, 0x80000, CRC(4079f342) SHA1(fa36aed1abbda54a42f29b183007474580870319) )
	ROM_LOAD32_BYTE( "cy1-poi-lu2.2l", 0x200002, 0x80000, CRC(61d816d4) SHA1(7991957b910d32530151abc7f469fcf1de62d8f3) )
	ROM_LOAD32_BYTE( "cy1-poi-ll2.2p", 0x200003, 0x80000, CRC(faf09158) SHA1(b56ebed6012362b1d599c396a43e90a1e4d9dc38) )

	ROM_REGION( 0x200000, "c140", 0 ) /* sound samples */
	ROM_LOAD("cy1-voi0.12b",0x000000,0x80000,CRC(99d7ce46) SHA1(b75f4055c3ce847daabfacda22df14e3f80c4fb9) )
	ROM_LOAD("cy1-voi1.12c",0x080000,0x80000,CRC(2b335f06) SHA1(2b2cd407c34388b56496f84a414daa153780b098) )
	ROM_LOAD("cy1-voi2.12d",0x100000,0x80000,CRC(10cd15f0) SHA1(9b721654ed97a13287373c1b2854ac9aeddc271f) )
	ROM_LOAD("cy1-voi3.12e",0x180000,0x80000,CRC(c902b4a4) SHA1(816357ec1a02a7ebf817ac1182e9c50ce5ca71f6) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "cybsled.nv",  0x000000, 0x2000, CRC(aa18bf9e) SHA1(3712d4d20e5f5f1c920e3f1f6a00101e874662d0) )
ROM_END

ROM_START( cybsledj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "cy1-mpr-u.3j",  0x000000, 0x80000, CRC(cc5a2e83) SHA1(b794051b2c351e9ca43351603845e4e563f6740f) )
	ROM_LOAD16_BYTE( "cy1-mpr-l.1j",  0x000001, 0x80000, CRC(f7ee8b48) SHA1(6d36eb3dba9cf7f5f5e1a26c156e77a2dad3f257) )

	ROM_REGION( 0x100000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "cy1-spr-u.6c",  0x000000, 0x80000, CRC(28dd707b) SHA1(11297ceae4fe78d170785a5cf9ad77833bbe7fff) )
	ROM_LOAD16_BYTE( "cy1-spr-l.4c",  0x000001, 0x80000, CRC(437029de) SHA1(3d275a2b0ce6909e77e657c371bd22597ea9d398) )

	ROM_REGION( 0x030000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "cy1-snd0.8j", 0x00c000, 0x004000, CRC(3dddf83b) SHA1(e16119cbef176b6f8f8ace773fcbc201e987823f) )
	ROM_CONTINUE(            0x010000, 0x01c000 )
	ROM_RELOAD(              0x010000, 0x020000 )

	ROM_REGION( 0x010000, "mcu", 0 ) /* I/O MCU */
	ROM_LOAD( "sys2mcpu.bin",  0x000000, 0x002000, CRC(a342a97e) SHA1(2c420d34dba21e409bf78ddca710fc7de65a6642) )
	ROM_LOAD( "sys2c65c.bin",  0x008000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x20000, "dspmaster", 0 ) /* Master DSP */
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )
	ROM_REGION( 0x20000, "dspslave", 0 ) /* Slave DSP */
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "cy1-obj0.5s",  0x000000, 0x80000, CRC(5ae542d5) SHA1(99b1a3ed476da4a97cb864538909d7b831f0fd3b) )
	ROM_LOAD( "cy1-obj4.4s",  0x080000, 0x80000, CRC(57904076) SHA1(b1dc0d99543bc4b9584b37ffc12c6ebc59e30e3b) )
	ROM_LOAD( "cy1-obj1.5x",  0x100000, 0x80000, CRC(4aae3eff) SHA1(c80240bd2f4228a0261a14adb6b10560b31b5aa0) )
	ROM_LOAD( "cy1-obj5.4x",  0x180000, 0x80000, CRC(0e11ca47) SHA1(076a9a4cfddbee2d8aaa06110333090d8fdbefeb) )
	ROM_LOAD( "cy1-obj2.3s",  0x200000, 0x80000, CRC(d64ec4c3) SHA1(0bed1cafc21ed8cef3850fb81e30076977086eb0) )
	ROM_LOAD( "cy1-obj6.2s",  0x280000, 0x80000, CRC(7748b485) SHA1(adb4da419a6cdbefd0fef182d866a3479be379af) )
	ROM_LOAD( "cy1-obj3.3x",  0x300000, 0x80000, CRC(3d1f7168) SHA1(392dddcc79fe61dcc6514a91ac27b5e36825d8b7) )
	ROM_LOAD( "cy1-obj7.2x",  0x380000, 0x80000, CRC(b6eb6ad2) SHA1(85a660c5e44012491be7d4e783cce6ba12c135cb) )

	ROM_REGION16_BE( 0x200000, "user1", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "cy1-data-u.3a",   0x000000, 0x80000, CRC(570da15d) SHA1(9ebe756f10756c079a92fb522332e9e52ff715c3) )
	ROM_LOAD16_BYTE( "cy1-data-l.1a",   0x000001, 0x80000, CRC(9cf96f9e) SHA1(91783f48b93e03c778c6641ca8fb419c13b0d3c5) )
	ROM_LOAD16_BYTE( "cy1-edata0-u.3b", 0x100000, 0x80000, CRC(77452533) SHA1(48fc199bcc1beb23c714eebd9b09b153c980170b) )
	ROM_LOAD16_BYTE( "cy1-edata0-l.1b", 0x100001, 0x80000, CRC(e812e290) SHA1(719e0a026ae8ef63d0d0269b67669ea9b4d950dd) )

	ROM_REGION32_BE( 0x400000, "user2", ROMREGION_ERASE )		/* 24bit signed point data */
	ROM_LOAD32_BYTE( "cy1-poi-h1.2f",  0x000001, 0x80000, CRC(eaf8bac3) SHA1(7a2caf6672af158b4a23ce4626342d1f17d1a4e4) )	/* most significant */
	ROM_LOAD32_BYTE( "cy1-poi-lu1.2k", 0x000002, 0x80000, CRC(c544a8dc) SHA1(4cce5f2ab3519b4aa7edbdd15b2d79a7fdcade3c) )
	ROM_LOAD32_BYTE( "cy1-poi-ll1.2n", 0x000003, 0x80000, CRC(30acb99b) SHA1(a28dcb3e5405f166644f6353a903c1143ee268f1) )	/* least significant */
	ROM_LOAD32_BYTE( "cy1-poi-h2.2j",  0x200001, 0x80000, CRC(4079f342) SHA1(fa36aed1abbda54a42f29b183007474580870319) )
	ROM_LOAD32_BYTE( "cy1-poi-lu2.2l", 0x200002, 0x80000, CRC(61d816d4) SHA1(7991957b910d32530151abc7f469fcf1de62d8f3) )
	ROM_LOAD32_BYTE( "cy1-poi-ll2.2p", 0x200003, 0x80000, CRC(faf09158) SHA1(b56ebed6012362b1d599c396a43e90a1e4d9dc38) )

	ROM_REGION( 0x200000, "c140", 0 ) /* sound samples */
	ROM_LOAD("cy1-voi0.12b",0x000000,0x80000,CRC(99d7ce46) SHA1(b75f4055c3ce847daabfacda22df14e3f80c4fb9) )
	ROM_LOAD("cy1-voi1.12c",0x080000,0x80000,CRC(2b335f06) SHA1(2b2cd407c34388b56496f84a414daa153780b098) )
	ROM_LOAD("cy1-voi2.12d",0x100000,0x80000,CRC(10cd15f0) SHA1(9b721654ed97a13287373c1b2854ac9aeddc271f) )
	ROM_LOAD("cy1-voi3.12e",0x180000,0x80000,CRC(c902b4a4) SHA1(816357ec1a02a7ebf817ac1182e9c50ce5ca71f6) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "cybsledj.nv",  0x000000, 0x2000, CRC(a73bb03e) SHA1(e074bfeae14178c867070e06f6690ed13115f5fa) )
ROM_END

ROM_START( driveyes )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* C68C - 68k code */
	ROM_LOAD16_BYTE( "de2-mp-ub.3j",  0x000000, 0x20000, CRC(f9c86fb5) SHA1(b48d16e8f26e7a2cfecb30285b517c42e5585ac7) )
	ROM_LOAD16_BYTE( "de2-mp-lb.1j",  0x000001, 0x20000, CRC(11d8587a) SHA1(ecb1e8fe2ba56b6f6a71a5552d5663b597165786) )

	ROM_REGION( 0x40000, "slave", 0 ) /* C68 - 68k code */
	ROM_LOAD16_BYTE( "de1-sp-ub.6c",  0x000000, 0x20000, CRC(231b144f) SHA1(42518614cb083455dc5fec71e699403907ca784b) )
	ROM_LOAD16_BYTE( "de1-sp-lb.4c",  0x000001, 0x20000, CRC(50cb9f59) SHA1(aec7fa080854f0297d9e90e3aaeb0f332fd579bd) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* Sound */
/*
There are 3 seperate complete boards used for this 3 screen version....
"Set2" (center screen board?) has de1_snd0 while the other 2 sets have de1_snd0r (rear speakers??)
Only "Set2" has voice roms present/dumped?
We load the "r" set, then load set2's sound CPU code over it to keep the "r" rom in the set
*/
	ROM_LOAD( "de1-snd0r.8j",  0x00c000, 0x004000, CRC(7bbeda42) SHA1(fe840cc9069758928492bbeec79acded18daafd9) ) /* Sets 1 & 3 */
	ROM_CONTINUE(              0x010000, 0x01c000 )
	ROM_RELOAD(                0x010000, 0x020000 )
	ROM_LOAD( "de1-snd0.8j",   0x00c000, 0x004000, CRC(5474f203) SHA1(e0ae2f6978deb0c934d9311a334a6e36bb402aee) ) /* Set 2 */
	ROM_CONTINUE(              0x010000, 0x01c000 )
	ROM_RELOAD(                0x010000, 0x020000 )

	ROM_REGION( 0x10000, "mcu", 0 ) /* I/O MCU */
	ROM_LOAD( "sys2mcpu.bin",  0x000000, 0x002000, CRC(a342a97e) SHA1(2c420d34dba21e409bf78ddca710fc7de65a6642) )
	ROM_LOAD( "sys2c65c.bin",  0x008000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x20000, "dsp", ROMREGION_ERASEFF ) /* C67 - DSP */

	ROM_REGION( 0x200000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "de1-obj0.5s",  0x000000, 0x40000, CRC(7438bd53) SHA1(7619c4b56d5c466e845eb45e6157dcaf2a03ad94) )
	ROM_LOAD( "de1-obj4.4s",  0x040000, 0x40000, CRC(335f0ea4) SHA1(9ec065d99ad0874b262b372334179a7e7612558e) )
	ROM_LOAD( "de1-obj1.5x",  0x080000, 0x40000, CRC(45f2334e) SHA1(95f277a4e43d6662ae44d6b69a57f65c72978319) )
	ROM_LOAD( "de1-obj5.4x",  0x0c0000, 0x40000, CRC(9e22999c) SHA1(02624186c359b5e2c96cd3f0e2cb1598ea36dff7) )
	ROM_LOAD( "de1-obj2.3s",  0x100000, 0x40000, CRC(8f1a542c) SHA1(2cb59713607d8929815a9b28bf2a384b6a6c9db8) )
	ROM_LOAD( "de1-obj6.2s",  0x140000, 0x40000, CRC(346df4d5) SHA1(edbadb9db93b7f5a3b064c7f6acb77001cdacce2) )
	ROM_LOAD( "de1-obj3.3x",  0x180000, 0x40000, CRC(fc94544c) SHA1(6297445c64784ee253716f6438d98e5fcd4e7520) )
	ROM_LOAD( "de1-obj7.2x",  0x1c0000, 0x40000, CRC(9ce325d7) SHA1(de4d788bec14842507ed405244974b4fd4f07515) )

	ROM_REGION16_BE( 0x100000, "user1", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "de1-data-u.3a",  0x00000, 0x80000, CRC(fe65d2ab) SHA1(dbe962dda7efa60357fa3a684a265aaad49df5b5) )
	ROM_LOAD16_BYTE( "de1-data-l.1a",  0x00001, 0x80000, CRC(9bb37aca) SHA1(7f5dffc95cadcf12f53ff7944920afc25ed3cf68) )

	ROM_REGION16_BE( 0xc0000, "user2", 0 ) /* 3d objects */
	ROM_LOAD16_BYTE( "de1-pt0-ub.8j", 0x00000, 0x20000, CRC(3b6b746d) SHA1(40c992ef4cf5187b30aba42c5fe7ce0f8f02bee0) )
	ROM_LOAD16_BYTE( "de1-pt0-lb.8d", 0x00001, 0x20000, CRC(9c5c477e) SHA1(c8ae8a663227d636d35bd5f432d23f05d6695942) )
	ROM_LOAD16_BYTE( "de1-pt1-u.8l",  0x40000, 0x20000, CRC(23bc72a1) SHA1(083e2955ae2f88d1ad461517b47054d64375b46e) )
	ROM_LOAD16_BYTE( "de1-pt1-l.8e",  0x40001, 0x20000, CRC(a05ee081) SHA1(1be4c61ad716abb809856e04d4bb450943706a55) )
	ROM_LOAD16_BYTE( "de1-pt2-u.5n",  0x80000, 0x20000, CRC(10e83d81) SHA1(446fedc3b1e258a39fb9467e5327c9f9a9f1ac3f) )
	ROM_LOAD16_BYTE( "de1-pt2-l.7n",  0x80001, 0x20000, CRC(3339a976) SHA1(c9eb9c04f7b3f2a85e5ab64ffb2fe4fcfb6c494b) )

	ROM_REGION( 0x200000, "c140", 0 ) /* sound samples */
	ROM_LOAD("de1-voi0.12b",  0x040000, 0x40000, CRC(fc44adbd) SHA1(4268bb1f025e47a94212351d1c1cfd0e5029221f) )
	ROM_LOAD("de1-voi1.12c",  0x0c0000, 0x40000, CRC(a71dc55a) SHA1(5e746184db9144ab4e3a97b20195b92b0f56c8cc) )
	ROM_LOAD("de1-voi2.12d",  0x140000, 0x40000, CRC(4d32879a) SHA1(eae65f4b98cee9efe4e5dad7298c3717cfb1e6bf) )
	ROM_LOAD("de1-voi3.12e",  0x1c0000, 0x40000, CRC(e4832d18) SHA1(0460c79d3942aab89a765b0bd8bbddaf19a6d682) )
ROM_END

ROM_START( starblad )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "st1-mp-u.bin",  0x000000, 0x80000, CRC(483a311c) SHA1(dd9416b8d4b0f8b361630e312eac71c113064eae) )
	ROM_LOAD16_BYTE( "st1-mp-l.bin",  0x000001, 0x80000, CRC(0a4dd661) SHA1(fc2b71a255a8613693c4d1c79ddd57a6d396165a) )

	ROM_REGION( 0x080000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "st1-sp-u.bin",  0x000000, 0x40000, CRC(9f9a55db) SHA1(72bf5d6908cc57cc490fa2292b4993d796b2974d) )
	ROM_LOAD16_BYTE( "st1-sp-l.bin",  0x000001, 0x40000, CRC(acbe39c7) SHA1(ca48b7ea619b1caaf590eed33001826ce7ef36d8) )

	ROM_REGION( 0x030000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "st1-snd0.bin", 0x00c000, 0x004000, CRC(c0e934a3) SHA1(678ed6705c6f494d7ecb801a4ef1b123b80979a5) )
	ROM_CONTINUE(             0x010000, 0x01c000 )
	ROM_RELOAD(               0x010000, 0x020000 )

	ROM_REGION( 0x010000, "mcu", 0 ) /* I/O MCU */
	ROM_LOAD( "sys2mcpu.bin",  0x000000, 0x002000, CRC(a342a97e) SHA1(2c420d34dba21e409bf78ddca710fc7de65a6642) )
	ROM_LOAD( "sys2c65c.bin",  0x008000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x20000, "dspmaster", 0 ) /* Master DSP */
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )
	ROM_REGION( 0x20000, "dspslave", 0 ) /* Slave DSP */
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "st1-obj0.bin",  0x000000, 0x80000, CRC(5d42c71e) SHA1(f1aa2bb31bbbcdcac8e94334b1c78238cac1a0e7) )
	ROM_LOAD( "st1-obj1.bin",  0x080000, 0x80000, CRC(c98011ad) SHA1(bc34c21428e0ef5887051c0eb0fdef5397823a82) )
	ROM_LOAD( "st1-obj2.bin",  0x100000, 0x80000, CRC(6cf5b608) SHA1(c8537fbe97677c4c8a365b1cf86c4645db7a7d6b) )
	ROM_LOAD( "st1-obj3.bin",  0x180000, 0x80000, CRC(cdc195bb) SHA1(91443917a6982c286b6f15381d441d061aefb138) )

	ROM_REGION16_BE( 0x40000, "user1", 0 )
	ROM_LOAD16_BYTE( "st1-data-u.bin",  0x000000, 0x20000, CRC(2433e911) SHA1(95f5f00d3bacda4996e055a443311fb9f9a5fe2f) )
	ROM_LOAD16_BYTE( "st1-data-l.bin",  0x000001, 0x20000, CRC(4a2cc252) SHA1(d9da9992bac878f8a1f5e84cc3c6d457b4705e8f) )

	ROM_REGION32_BE( 0x400000, "user2", ROMREGION_ERASE ) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "st1-pt0-h.bin", 0x000001, 0x80000, CRC(84eb355f) SHA1(89a248b8be2e0afcee29ba4c4c9cca65d5fb246a) )
	ROM_LOAD32_BYTE( "st1-pt0-u.bin", 0x000002, 0x80000, CRC(1956cd0a) SHA1(7d21b3a59f742694de472c545a1f30c3d92e3390) )
	ROM_LOAD32_BYTE( "st1-pt0-l.bin", 0x000003, 0x80000, CRC(ff577049) SHA1(1e1595174094e88d5788753d05ce296c1f7eca75) )
	ROM_LOAD32_BYTE( "st1-pt1-h.bin", 0x200001, 0x80000, CRC(96b1bd7d) SHA1(55da7896dda2aa4c35501a55c8605a065b02aa17) )
	ROM_LOAD32_BYTE( "st1-pt1-u.bin", 0x200002, 0x80000, CRC(ecf21047) SHA1(ddb13f5a2e7d192f0662fa420b49f89e1e991e66) )
	ROM_LOAD32_BYTE( "st1-pt1-l.bin", 0x200003, 0x80000, CRC(01cb0407) SHA1(4b58860bbc353de8b4b8e83d12b919d9386846e8) )

	ROM_REGION( 0x200000, "c140", 0 ) /* sound samples */
	ROM_LOAD("st1-voi0.bin",0x000000,0x80000,CRC(5b3d43a9) SHA1(cdc04f19dc91dca9fa88ba0c2fca72aa195a3694) )
	ROM_LOAD("st1-voi1.bin",0x080000,0x80000,CRC(413e6181) SHA1(e827ec11f5755606affd2635718512aeac9354da) )
	ROM_LOAD("st1-voi2.bin",0x100000,0x80000,CRC(067d0720) SHA1(a853b2d43027a46c5e707fc677afdaae00f450c7) )
	ROM_LOAD("st1-voi3.bin",0x180000,0x80000,CRC(8b5aa45f) SHA1(e1214e639200758ad2045bde0368a2d500c1b84a) )
ROM_END

ROM_START( solvalou )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "sv1-mp-u.bin",  0x000000, 0x20000, CRC(b6f92762) SHA1(d177328b3da2ab0580e101478142bc8c373d6140) )
	ROM_LOAD16_BYTE( "sv1-mp-l.bin",  0x000001, 0x20000, CRC(28c54c42) SHA1(32fcca2eb4bb8ba8c2587b03d3cf59f072f7fac5) )

	ROM_REGION( 0x80000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "sv1-sp-u.bin",  0x000000, 0x20000, CRC(ebd4bf82) SHA1(67946360d680a675abcb3c131bac0502b2455573) )
	ROM_LOAD16_BYTE( "sv1-sp-l.bin",  0x000001, 0x20000, CRC(7acab679) SHA1(764297c9601be99dbbffb75bbc6fe4a40ea38529) )

	ROM_REGION( 0x030000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "sv1-snd0.bin", 0x00c000, 0x004000, CRC(5e007864) SHA1(94da2d51544c6127056beaa251353038646da15f) )
	ROM_CONTINUE(             0x010000, 0x01c000 )
	ROM_RELOAD(               0x010000, 0x020000 )

	ROM_REGION( 0x010000, "mcu", 0 ) /* I/O MCU */
	ROM_LOAD( "sys2mcpu.bin",  0x000000, 0x002000, CRC(a342a97e) SHA1(2c420d34dba21e409bf78ddca710fc7de65a6642) )
	ROM_LOAD( "sys2c65c.bin",  0x008000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x20000, "dspmaster", 0 ) /* Master DSP */
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )
	ROM_REGION( 0x20000, "dspslave", 0 ) /* Slave DSP */
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "sv1-obj0.bin",  0x000000, 0x80000, CRC(773798bb) SHA1(51ab76c95030bab834f1a74ae677b2f0afc18c52) )
	ROM_LOAD( "sv1-obj4.bin",  0x080000, 0x80000, CRC(33a008a7) SHA1(4959a0ac24ad64f1367e2d8d63d39a0273c60f3e) )
	ROM_LOAD( "sv1-obj1.bin",  0x100000, 0x80000, CRC(a36d9e79) SHA1(928d9995e97ee7509e23e6cc64f5e7bfb5c02d42) )
	ROM_LOAD( "sv1-obj5.bin",  0x180000, 0x80000, CRC(31551245) SHA1(385452ea4830c466263ad5241313ac850dfef756) )
	ROM_LOAD( "sv1-obj2.bin",  0x200000, 0x80000, CRC(c8672b8a) SHA1(8da037b27d2c2b178aab202781f162371458f788) )
	ROM_LOAD( "sv1-obj6.bin",  0x280000, 0x80000, CRC(fe319530) SHA1(8f7e46c8f0b86c7515f6d763b795ce07d11c77bc) )
	ROM_LOAD( "sv1-obj3.bin",  0x300000, 0x80000, CRC(293ef1c5) SHA1(f677883bfec16bbaeb0a01ac565d0e6cac679174) )
	ROM_LOAD( "sv1-obj7.bin",  0x380000, 0x80000, CRC(95ed6dcb) SHA1(931706ce3fea630823ce0c79febec5eec0cc623d) )

	ROM_REGION16_BE( 0x100000, "user1", 0 )
	ROM_LOAD16_BYTE( "sv1-data-u.bin",  0x000000, 0x80000, CRC(2e561996) SHA1(982158481e5649f21d5c2816fdc80cb725ed1419) )
	ROM_LOAD16_BYTE( "sv1-data-l.bin",  0x000001, 0x80000, CRC(495fb8dd) SHA1(813d1da4109652008d72b3bdb03032efc5c0c2d5) )

	ROM_REGION32_BE( 0x400000, "user2", ROMREGION_ERASE )		/* 24bit signed point data */
	ROM_LOAD32_BYTE( "sv1-pt0-h.bin", 0x000001, 0x80000, CRC(3be21115) SHA1(c9f30353c1216f64199f87cd34e787efd728e739) ) /* most significant */
	ROM_LOAD32_BYTE( "sv1-pt0-u.bin", 0x000002, 0x80000, CRC(4aacfc42) SHA1(f0e179e057183b41744ca429764f44306f0ce9bf) )
	ROM_LOAD32_BYTE( "sv1-pt0-l.bin", 0x000003, 0x80000, CRC(6a4dddff) SHA1(9ed182d21d328c6a684ee6658a9dfcf3f3dd8646) ) /* least significant */

	ROM_REGION( 0x200000, "c140", 0 ) /* sound samples */
	ROM_LOAD("sv1-voi0.bin",0x000000,0x80000,CRC(7f61bbcf) SHA1(b3b7e66e24d9cb16ebd139237c1e51f5d60c1585) )
	ROM_LOAD("sv1-voi1.bin",0x080000,0x80000,CRC(c732e66c) SHA1(14e75dd9bea4055f85eb2bcbf69cf6695a3f7ec4) )
	ROM_LOAD("sv1-voi2.bin",0x100000,0x80000,CRC(51076298) SHA1(ec52c9ae3029118f3ea3732948d6de28f5fba561) )
	ROM_LOAD("sv1-voi3.bin",0x180000,0x80000,CRC(33085ff3) SHA1(0a30b91618c250a5e7bd896a8ceeb3d16da178a9) )
ROM_END

ROM_START( winrun )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "sg1-mp-ub.3k",  0x000000, 0x20000, CRC(7f9b855a) SHA1(6d39a3a9959dbcd0047dbaab0fcd68adc81f5508) )
	ROM_LOAD16_BYTE( "sg1-mp-lb.1k",  0x000001, 0x20000, CRC(a45e8543) SHA1(f9e583a988e4661026ee7873a48d078225778df3) )

	ROM_REGION( 0x40000, "slave", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "sg1-sp-u.6b",  0x000000, 0x20000, CRC(7c9c3a3f) SHA1(cacb45c9111ac66c6e60b7a0cacd8bf47fd00752) )
	ROM_LOAD16_BYTE( "sg1-sp-l.4b",  0x000001, 0x20000, CRC(5068fc5d) SHA1(7f6e80f74985959509d824318a4a7ff2b11953da) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "sg1-snd0.7c", 0x00c000, 0x004000, CRC(de04b794) SHA1(191f4d79ac2375d7060f3d83ec753185e92f28ea) )
	ROM_CONTINUE(            0x010000, 0x01c000 )
	ROM_RELOAD(              0x010000, 0x020000 )

	ROM_REGION( 0x10000, "mcu", 0 ) /* I/O MCU */
	ROM_LOAD( "sys2mcpu.bin",  0x000000, 0x002000, CRC(a342a97e) SHA1(2c420d34dba21e409bf78ddca710fc7de65a6642) )
	ROM_LOAD( "sys2c65c.bin",  0x008000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x20000, "dsp", ROMREGION_ERASEFF ) /* DSP */

	ROM_REGION( 0x80000, "gpu", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "sg1-gp0-u.1j",  0x00000, 0x20000, CRC(475da78a) SHA1(6e69bcc6caf2e3cd28fed75796c8992e754f9323) )
	ROM_LOAD16_BYTE( "sg1-gp0-l.3j",  0x00001, 0x20000, CRC(580479bf) SHA1(ba682190cba0d3cdc49aa4937c898ba7ed2a25f5) )
	ROM_LOAD16_BYTE( "sg1-gp1-u.1l",  0x40000, 0x20000, CRC(f5f2e927) SHA1(ebf709f16f01f1a634de9121454537cda74e891b) )
	ROM_LOAD16_BYTE( "sg1-gp1-l.3l",  0x40001, 0x20000, CRC(17ed90a5) SHA1(386bdcb11dcbe400f5be1fe4a7418158b46e50ef) )

	ROM_REGION16_BE( 0x80000, "user1", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "sg1-data0-u.3a",  0x00000, 0x20000, CRC(1dde2ac2) SHA1(2d20a434561c04e48b52a2137a8c9047e17c1013) )
	ROM_LOAD16_BYTE( "sg1-data0-l.1a",  0x00001, 0x20000, CRC(2afeb77e) SHA1(ac1552f6e2788158d3477b6a0981d001d6cbdf13) )
	ROM_LOAD16_BYTE( "sg1-data1-u.3b",  0x40000, 0x20000, CRC(5664b09e) SHA1(10c1c29614eee2cffcfd69085f0450d81ba2e25f) )
	ROM_LOAD16_BYTE( "sg1-data1-l.1b",  0x40001, 0x20000, CRC(2dbc7de4) SHA1(824304c95942c7296f8e8dcf8ee7e22bf56154b1) )

	ROM_REGION16_BE( 0x80000, "user2", 0 ) /* 3d objects */
	ROM_LOAD16_BYTE( "sg1-pt0-u.8j", 0x00000, 0x20000, CRC(160c3634) SHA1(485d20d6cc459f17d77682201dee07bdf76bf343) )
	ROM_LOAD16_BYTE( "sg1-pt0-l.8d", 0x00001, 0x20000, CRC(b5a665bf) SHA1(5af6ec492f31395c0492e14590b025b120067b8d) )
	ROM_LOAD16_BYTE( "sg1-pt1-u.8l", 0x40000, 0x20000, CRC(b63d3006) SHA1(78e78619766b0fd91b1e830cfb066495d6773981) )
	ROM_LOAD16_BYTE( "sg1-pt1-l.8e", 0x40001, 0x20000, CRC(6385e325) SHA1(d50bceb2e9c0d0a38d7b0f918f99c482649e260d) )

	ROM_REGION16_BE( 0x100000, "user3", 0 ) /* bitmapped graphics */
	ROM_LOAD16_BYTE( "sg1-gd0-u.1p",  0x00000, 0x40000, CRC(7838fcde) SHA1(45e31269eed1999b73c41c2f5d2c5bfbbdaf23df) )
	ROM_LOAD16_BYTE( "sg1-gd0-l.3p",  0x00001, 0x40000, CRC(4bd02b9a) SHA1(b2fdfd1c1325864aaad87f5358ab9bbdd79ff6ae) )
	ROM_LOAD16_BYTE( "sg1-gd1-u.1s",  0x80000, 0x40000, CRC(271db29b) SHA1(8b35fcf273b9aec28d4c606c41c0626dded697e1) )
	ROM_LOAD16_BYTE( "sg1-gd1-l.3s",  0x80001, 0x40000, CRC(a6c4da96) SHA1(377dbf21a1bede01de16708c96c112abab4417ce) )

	ROM_REGION( 0x200000, "c140", 0 ) /* sound samples */
	ROM_LOAD("sg-voi-1.11c",0x080000,0x80000,CRC(7dcccb31) SHA1(4441b37691434b13eae5dee2d04dc12a56b04d2a) )
	ROM_LOAD("sg-voi-3.11e",0x180000,0x80000,CRC(a198141c) SHA1(b4ca352e6aedd9d7a7e5e39e840f1d3a7145900e) )
ROM_END

ROM_START( winrun91 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "mpu.3k",  0x000000, 0x20000, CRC(80a0e5be) SHA1(6613b95e164c2032ea9043e4161130c6b3262492) )
	ROM_LOAD16_BYTE( "mpl.1k",  0x000001, 0x20000, CRC(942172d8) SHA1(21d8dfd2165b5ceb0399fdb53d9d0f51f1255803) )

	ROM_REGION( 0x40000, "slave", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "spu.6b",  0x000000, 0x20000, CRC(0221d4b2) SHA1(65fd38b1cfaa6693d71248561d764a9ea1098c56) )
	ROM_LOAD16_BYTE( "spl.4b",  0x000001, 0x20000, CRC(288799e2) SHA1(2c4bf0cf9c71458fff4dd77e426a76685d9e1bab) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "snd0.7c", 0x00c000, 0x004000, CRC(6a321e1e) SHA1(b2e77cac4ed7609593fa5a462c9d78526451e477) )
	ROM_CONTINUE(        0x010000, 0x01c000 )
	ROM_RELOAD(          0x010000, 0x020000 )

	ROM_REGION( 0x10000, "mcu", 0 ) /* I/O MCU */
	ROM_LOAD( "sys2mcpu.bin",  0x000000, 0x002000, CRC(a342a97e) SHA1(2c420d34dba21e409bf78ddca710fc7de65a6642) )
	ROM_LOAD( "sys2c65c.bin",  0x008000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x20000, "dsp", ROMREGION_ERASEFF ) /* DSP */

	ROM_REGION( 0x80000, "gpu", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "gp0u.1j",  0x00000, 0x20000, CRC(f5469a29) SHA1(38b6ea1fbe482b69fbb0e2f44f44a0ca2a49f6bc) )
	ROM_LOAD16_BYTE( "gp0l.3j",  0x00001, 0x20000, CRC(5c18f596) SHA1(215cbda62254e31b4ff6431623384df1639bfdb7) )
	ROM_LOAD16_BYTE( "gp1u.1l",  0x40000, 0x20000, CRC(146ab6b8) SHA1(aefb89585bf311f8d33f18298fea326ef1f19f1e) )
	ROM_LOAD16_BYTE( "gp1l.3l",  0x40001, 0x20000, CRC(96c2463c) SHA1(e43db580e7b454af04c22e894108fbb56da0eeb5) )

	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "d0u.3a",  0x00000, 0x20000, CRC(dcb27da5) SHA1(ecd72397d10313fe8dcb8589bdc5d88d4298b26c) )
	ROM_LOAD16_BYTE( "d0l.1a",  0x00001, 0x20000, CRC(f692a8f3) SHA1(4c29f60400b18d9ef0425de149618da6cf762ca4) )
	ROM_LOAD16_BYTE( "d1u.3b",  0x40000, 0x20000, CRC(ac2afd1b) SHA1(510eb41931164b086c85ba0a86d6f10b88f5e534) )
	ROM_LOAD16_BYTE( "d1l.1b",  0x40001, 0x20000, CRC(ebb51af1) SHA1(87b7b64ee662bf652add1e1199e42391d0e2f7e8) )

	ROM_REGION16_BE( 0x80000, "user2", 0 ) /* winrun91 - 3d objects */
	ROM_LOAD16_BYTE( "pt0u.8j", 0x00000, 0x20000, CRC(abf512a6) SHA1(e86288039d6c4dedfa95b11cb7e4b87637f90c09) )
	ROM_LOAD16_BYTE( "pt0l.8d", 0x00001, 0x20000, CRC(ac8d468c) SHA1(d1b457a19a5d3259d0caf933f42b3a02b485867b) )
	ROM_LOAD16_BYTE( "pt1u.8l", 0x40000, 0x20000, CRC(7e5dab74) SHA1(5bde219d5b4305d38d17b494b2e759f05d05329f) )
	ROM_LOAD16_BYTE( "pt1l.8e", 0x40001, 0x20000, CRC(38a54ec5) SHA1(5c6017c98cae674868153ff2d64532027cf0ab83) )

	ROM_REGION16_BE( 0x100000, "user3", 0 ) /* bitmapped graphics */
	ROM_LOAD16_BYTE( "gd0u.1p",  0x00000, 0x40000, CRC(33f5a19b) SHA1(b1dbd242168007f80e13e11c78b34abc1668883e) )
	ROM_LOAD16_BYTE( "gd0l.3p",  0x00001, 0x40000, CRC(9a29500e) SHA1(c605f86b138e0a4c3163ffd967482e298a15fbe7) )
	ROM_LOAD16_BYTE( "gd1u.1s",  0x80000, 0x40000, CRC(17e5a61c) SHA1(272ebd7daa56847f1887809535362331b5465dec) )
	ROM_LOAD16_BYTE( "gd1l.3s",  0x80001, 0x40000, CRC(64df59a2) SHA1(1e9d0945b94780bb0be16803e767466d2cda07e8) )

	ROM_REGION( 0x200000, "c140", 0 ) /* sound samples */
	ROM_LOAD("avo1.11c",0x080000,0x80000,CRC(9fb33af3) SHA1(666630a8e5766ca4c3275961963c3e713dfdda2d) )
	ROM_LOAD("avo3.11e",0x180000,0x80000,CRC(76e22f92) SHA1(0e1b8d35a5b9c20cc3192d935f0c9da1e69679d2) )
ROM_END

static void namcos21_init( running_machine *machine, int game_type )
{
	namcos2_gametype = game_type;
	pointram = auto_alloc_array(machine, UINT8, PTRAM_SIZE);
	mpDataROM = (UINT16 *)memory_region( machine, "user1" );
	InitDSP(machine);
	mbNeedsKickstart = 20;
	if( game_type==NAMCOS21_CYBERSLED )
	{
		mbNeedsKickstart = 200;
	}
} /* namcos21_init */

static DRIVER_INIT( winrun )
{
	UINT16 *pMem = (UINT16 *)memory_region(machine, "dsp");
	int pc = 0;
	pMem[pc++] = 0xff80; /* b */
	pMem[pc++] = 0;

	winrun_dspcomram = auto_alloc_array(machine, UINT16, 0x1000*2);

	namcos2_gametype = NAMCOS21_WINRUN91;
	mpDataROM = (UINT16 *)memory_region( machine, "user1" );
	pointram = auto_alloc_array(machine, UINT8, PTRAM_SIZE);
	pointram_idx = 0;
	mbNeedsKickstart = 0;
}

static DRIVER_INIT( aircombt )
{
	namcos21_init( machine, NAMCOS21_AIRCOMBAT );
}

static DRIVER_INIT( starblad )
{
	namcos21_init( machine, NAMCOS21_STARBLADE );
}


static DRIVER_INIT( cybsled )
{
	namcos21_init( machine, NAMCOS21_CYBERSLED );
}

static DRIVER_INIT( solvalou )
{
	UINT16 *mem = (UINT16 *)memory_region(machine, "maincpu");
	mem[0x20ce4/2+1] = 0x0000; // $200128
	mem[0x20cf4/2+0] = 0x4e71; // 2nd ptr_booting
	mem[0x20cf4/2+1] = 0x4e71;
	mem[0x20cf4/2+2] = 0x4e71;

	namcos21_init( machine, NAMCOS21_SOLVALOU );
}

static DRIVER_INIT( driveyes )
{
	UINT16 *pMem = (UINT16 *)memory_region(machine, "dsp");
	int pc = 0;
	pMem[pc++] = 0xff80; /* b */
	pMem[pc++] = 0;
	winrun_dspcomram = auto_alloc_array(machine, UINT16, 0x1000*2);
	namcos2_gametype = NAMCOS21_DRIVERS_EYES;
	mpDataROM = (UINT16 *)memory_region( machine, "user1" );
	pointram = auto_alloc_array(machine, UINT8, PTRAM_SIZE);
	pointram_idx = 0;
	mbNeedsKickstart = 0;
}

/*************************************************************/
/*                                                           */
/*  NAMCO SYSTEM 21 INPUT PORTS                              */
/*                                                           */
/*************************************************************/

static INPUT_PORTS_START( s21default )
	PORT_START("PORTB")		/* 63B05Z0 - PORT B */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("PORTC")		/* 63B05Z0 - PORT C & SCI */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_0) PORT_TOGGLE // alt test mode switch
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("AN0")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN1")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x60,0x9f) PORT_SENSITIVITY(15) PORT_KEYDELTA(10)
	PORT_START("AN2")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x60,0x9f) PORT_SENSITIVITY(20) PORT_KEYDELTA(10)
	PORT_START("AN3")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN4")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN5")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 5 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN6")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 6 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN7")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 7 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PORTH")		/* 63B05Z0 - PORT H */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")		/* 63B05Z0 - $2000 DIP SW */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "DSW2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW3")
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW4")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW5")
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW6")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW7")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("DIAL0")		/* 63B05Z0 - $3000 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("DIAL1")		/* 63B05Z0 - $3001 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("DIAL2")		/* 63B05Z0 - $3002 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("DIAL3")		/* 63B05Z0 - $3003 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* "SCI - ? */
static INPUT_PORTS_START( winrun )
	PORT_INCLUDE(s21default)

	PORT_MODIFY("PORTB")		/* 63B05Z0 - PORT B */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 ) /* ? */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) /* ? */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) /* ? */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) /* ? */

	PORT_MODIFY("AN0")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN1")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(15) PORT_KEYDELTA(10) /* gas */
	PORT_MODIFY("AN2")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(15) PORT_KEYDELTA(10) /* steering */
	PORT_MODIFY("AN3")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(15) PORT_KEYDELTA(10) /* break */
	PORT_MODIFY("AN4")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN5")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 5 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN6")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 6 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN7")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 7 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTH")		/* 63B05Z0 - PORT H */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* shift down */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* shift up */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// the default inc/dec analog keys have been chosen to map 'tank' style inputs found on Assault.
// this makes the game easier to use with the keyboard, providing a familiar left/right stick mapping
// ports are limited to 10/ef because otherwise, even when calibrated, the game will act as if the
// inputs wrap around when they hit the maximum, causing undesired movement
static INPUT_PORTS_START( cybsled )
	PORT_INCLUDE(s21default)

	PORT_MODIFY("AN0")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xef) /* using 0x00 / 0xff causes controls to malfunction */ PORT_CODE_DEC(KEYCODE_I) PORT_CODE_INC(KEYCODE_K) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) /* right joystick: vertical */
	PORT_MODIFY("AN1")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xef) /* using 0x00 / 0xff causes controls to malfunction */ PORT_CODE_DEC(KEYCODE_E) PORT_CODE_INC(KEYCODE_D) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) /* left joystick: vertical */
	PORT_MODIFY("AN2")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xef) /* using 0x00 / 0xff causes controls to malfunction */ PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_L) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) /* right joystick: horizontal */
	PORT_MODIFY("AN3")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xef) /* using 0x00 / 0xff causes controls to malfunction */ PORT_CODE_DEC(KEYCODE_S) PORT_CODE_INC(KEYCODE_F) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) /* left joystick: horizontal */
	PORT_MODIFY("AN4")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN5")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 5 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN6")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 6 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN7")		/* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 7 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTH")		/* 63B05Z0 - PORT H */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( aircombt )
	PORT_INCLUDE(s21default)

	PORT_MODIFY("AN0")		/* IN#2: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN1")		/* IN#3: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)
	PORT_MODIFY("AN2")		/* IN#4: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)
	PORT_MODIFY("AN3")		/* IN#5: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_PLAYER(2)
	PORT_MODIFY("AN4")		/* IN#6: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN5")		/* IN#7: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 5 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN6")		/* IN#8: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 6 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN7")		/* IN#9: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 7 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")		/* 63B05Z0 - $2000 DIP SW */
	PORT_DIPNAME( 0x01, 0x01, "DSW1") // not test mode on this gamef
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_MODIFY("PORTH")		/* IN#10: 63B05Z0 - PORT H */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) ///???
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) // prev color
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) // ???next color
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*    YEAR, NAME,     PARENT,  MACHINE,           INPUT,        INIT,     MONITOR,  COMPANY,   FULLNAME,                       FLAGS */
GAME( 1992, aircomb,  0,       poly_c140_typeB,   aircombt,     aircombt, ROT0,    "Namco", "Air Combat (US)",	             GAME_NOT_WORKING|GAME_IMPERFECT_GRAPHICS )
GAME( 1992, aircombj, aircomb, poly_c140_typeB,   aircombt,     aircombt, ROT0,    "Namco", "Air Combat (Japan)",            GAME_NOT_WORKING|GAME_IMPERFECT_GRAPHICS )
GAME( 1993, cybsled,  0,       poly_c140_typeA,   cybsled,      cybsled,  ROT0,    "Namco", "Cyber Sled (US)",               GAME_IMPERFECT_GRAPHICS )
GAME( 1993, cybsledj, cybsled, poly_c140_typeA,   cybsled,      cybsled,  ROT0,    "Namco", "Cyber Sled (Japan)",            GAME_IMPERFECT_GRAPHICS )
/* 1992, ShimDrive */
GAME( 1991, solvalou, 0,       poly_c140_typeA,   s21default,   solvalou, ROT0,    "Namco", "Solvalou (Japan)",              GAME_IMPERFECT_GRAPHICS )
GAME( 1991, starblad, 0,       poly_c140_typeA,   s21default,   starblad, ROT0,    "Namco", "Starblade (Japan)",                     GAME_IMPERFECT_GRAPHICS )
GAME( 1991, winrun91, 0,       winrun_c140_typeB, winrun,       winrun,   ROT0,    "Namco", "Winning Run 91 (Japan)",                GAME_NOT_WORKING|GAME_IMPERFECT_GRAPHICS )
GAME( 1989, winrun,   0,       winrun_c140_typeB, winrun,       winrun,   ROT0,    "Namco", "Winning Run Suzuka Grand Prix (Japan)", GAME_NOT_WORKING|GAME_IMPERFECT_GRAPHICS )
/* 1988, Winning Run */
GAME( 1991, driveyes, 0,       driveyes,          winrun,       driveyes, ROT0,    "Namco", "Driver's Eyes (US)",                 GAME_NOT_WORKING|GAME_IMPERFECT_GRAPHICS )
