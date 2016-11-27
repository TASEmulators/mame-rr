/* video/namcofl.c */

#include "emu.h"
#include "includes/namconb1.h"
#include "includes/namcoic.h"
#include "includes/namcos2.h"

//UINT32 *namcofl_spritebank32;
//UINT32 *namcofl_tilebank32;

static UINT32 namcofl_sprbank;

/* nth_word32 is a general-purpose utility function, which allows us to
 * read from 32-bit aligned memory as if it were an array of 16 bit words.
 */
#ifdef UNUSED_FUNCTION
INLINE UINT16
nth_word32( const UINT32 *source, int which )
{
	source += which/2;
	which ^= 1;	/* i960 is little-endian */
	if( which&1 )
	{
		return (*source)&0xffff;
	}
	else
	{
		return (*source)>>16;
	}
}
#endif

/* nth_byte32 is a general-purpose utility function, which allows us to
 * read from 32-bit aligned memory as if it were an array of bytes.
 */
#ifdef UNUSED_FUNCTION
INLINE UINT8
nth_byte32( const UINT32 *pSource, int which )
{
		UINT32 data = pSource[which/4];

		which ^= 3;	/* i960 is little-endian */
		switch( which&3 )
		{
		case 0: return data>>24;
		case 1: return (data>>16)&0xff;
		case 2: return (data>>8)&0xff;
		default: return data&0xff;
		}
} /* nth_byte32 */
#endif

static void namcofl_install_palette(running_machine *machine)
{
	int pen, page, dword_offset, byte_offset;
	UINT32 r,g,b;
	UINT32 *pSource;

	/* this is unnecessarily expensive.  Better would be to mark palette entries dirty as
     * they are modified, and only process those that have changed.
     */
	pen = 0;
	for( page=0; page<4; page++ )
	{
		pSource = &machine->generic.paletteram.u32[page*0x2000/4];
		for( dword_offset=0; dword_offset<0x800/4; dword_offset++ )
		{
			r = pSource[dword_offset+0x0000/4];
			g = pSource[dword_offset+0x0800/4];
			b = pSource[dword_offset+0x1000/4];

			for( byte_offset=0; byte_offset<4; byte_offset++ )
			{
				palette_set_color_rgb( machine, pen++, r&0xff, g&0xff, b&0xff);
				r>>=8; g>>=8; b>>=8;
			}
		}
	}
}
static void TilemapCB(UINT16 code, int *tile, int *mask )
{
	*tile = code;
	*mask = code;
}


VIDEO_UPDATE( namcofl )
{
	int pri;

	namcofl_install_palette(screen->machine);

	bitmap_fill( bitmap, cliprect , get_black_pen(screen->machine));

	for( pri=0; pri<16; pri++ )
	{
		namco_roz_draw( bitmap,cliprect,pri );
		if((pri&1)==0)
			namco_tilemap_draw( bitmap, cliprect, pri>>1 );
		namco_obj_draw(screen->machine, bitmap, cliprect, pri );
	}

	return 0;
}

// NOTE : The two low bits toggle banks (code + 0x4000) for two
//        groups of sprites.  I am unsure how to differentiate those groups
//        at this time however.

WRITE32_HANDLER(namcofl_spritebank_w)
{
	COMBINE_DATA(&namcofl_sprbank);
}

static int FLobjcode2tile( int code )
{
	if ((code & 0x2000) && (namcofl_sprbank & 2)) { code += 0x4000; }

	return code;
}

VIDEO_START( namcofl )
{
	namco_tilemap_init( machine, NAMCONB1_TILEGFX, memory_region(machine, NAMCONB1_TILEMASKREGION), TilemapCB );
	namco_obj_init(machine,NAMCONB1_SPRITEGFX,0x0,FLobjcode2tile);
	namco_roz_init(machine,NAMCONB1_ROTGFX,NAMCONB1_ROTMASKREGION);
}
