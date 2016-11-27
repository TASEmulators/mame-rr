/* video hardware for Namco System II */

#include "emu.h"

#include "includes/namcos2.h"
#include "includes/namcoic.h"

UINT16 *namcos2_sprite_ram;
UINT16 *namcos2_68k_palette_ram;
size_t namcos2_68k_palette_size;
//size_t namcos2_68k_roz_ram_size;
UINT16 *namcos2_68k_roz_ram;

static UINT16 namcos2_68k_roz_ctrl[0x8];
static tilemap_t *tilemap_roz;

static void
TilemapCB( UINT16 code, int *tile, int *mask )
{
	*mask = code;

	switch( namcos2_gametype )
	{
	case NAMCOS2_FINAL_LAP_2:
	case NAMCOS2_FINAL_LAP_3:
		*tile = (code&0x07ff)|((code&0x4000)>>3)|((code&0x3800)<<1);
		break;

	default:
		/* The order of bits needs to be corrected to index the right tile  14 15 11 12 13 */
		*tile = (code&0x07ff)|((code&0xc000)>>3)|((code&0x3800)<<2);
		break;
	}
} /* TilemapCB */

/**
 * namcos2_gfx_ctrl selects a bank of 128 sprites within spriteram
 *
 * namcos2_gfx_ctrl also supplies palette and priority information that is applied to the output of the
 *                  Namco System 2 ROZ chip
 *
 * -xxx ---- ---- ---- roz priority
 * ---- xxxx ---- ---- roz palette
 * ---- ---- xxxx ---- always zero?
 * ---- ---- ---- xxxx sprite bank
 */
static UINT16 namcos2_gfx_ctrl;

READ16_HANDLER( namcos2_gfx_ctrl_r )
{
	return namcos2_gfx_ctrl;
} /* namcos2_gfx_ctrl_r */

WRITE16_HANDLER( namcos2_gfx_ctrl_w )
{
	COMBINE_DATA(&namcos2_gfx_ctrl);
} /* namcos2_gfx_ctrl_w */

static TILE_GET_INFO( get_tile_info_roz )
{
	int tile = namcos2_68k_roz_ram[tile_index];
	SET_TILE_INFO(3,tile,0/*color*/,0);
} /* get_tile_info_roz */

struct RozParam
{
	UINT32 size;
	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;
	int color;
	int wrap;
};

INLINE void
DrawRozHelperBlock(const struct RozParam *rozInfo, int destx, int desty,
	int srcx, int srcy, int width, int height,
	bitmap_t *destbitmap, bitmap_t *flagsbitmap,
	bitmap_t *srcbitmap, UINT32 size_mask)
{
	int desty_end = desty + height;

	int end_incrx = rozInfo->incyx - (width * rozInfo->incxx);
	int end_incry = rozInfo->incyy - (width * rozInfo->incxy);

	UINT16 *dest = BITMAP_ADDR16(destbitmap, desty, destx);
	int dest_rowinc = destbitmap->rowpixels - width;

	while (desty < desty_end)
	{
		UINT16 *dest_end = dest + width;
		while (dest < dest_end)
		{
			UINT32 xpos = (srcx >> 16);
			UINT32 ypos = (srcy >> 16);

			if (rozInfo->wrap)
			{
				xpos &= size_mask;
				ypos &= size_mask;
			}
			else if ((xpos > rozInfo->size) || (ypos >= rozInfo->size))
			{
				goto L_SkipPixel;
			}

			if (*BITMAP_ADDR8(flagsbitmap, ypos, xpos) & TILEMAP_PIXEL_LAYER0)
			{
				*dest = *BITMAP_ADDR16(srcbitmap, ypos, xpos) + rozInfo->color;
			}

		L_SkipPixel:

			srcx += rozInfo->incxx;
			srcy += rozInfo->incxy;
			dest++;
		}
		srcx += end_incrx;
		srcy += end_incry;
		dest += dest_rowinc;
		desty++;
	}
} /* DrawRozHelperBlock */

static void
DrawRozHelper(
	bitmap_t *bitmap,
	tilemap_t *tmap,
	const rectangle *clip,
	const struct RozParam *rozInfo )
{
	tilemap_set_palette_offset( tmap, rozInfo->color );

	if( bitmap->bpp == 16 )
	{
		/* On many processors, the simple approach of an outer loop over the
            rows of the destination bitmap with an inner loop over the columns
            of the destination bitmap has poor performance due to the order
            that memory in the source bitmap is referenced when rotation
            approaches 90 or 270 degrees.  The reason is that the inner loop
            ends up reading pixels not sequentially in the source bitmap, but
            instead at rozInfo->incxx increments, which is at its maximum at 90
            degrees of rotation.  This means that only a few (or as few as
            one) source pixels are in each cache line at a time.

            Instead of the above, this code iterates in NxN blocks through the
            destination bitmap.  This has more overhead when there is little or
            no rotation, but much better performance when there is closer to 90
            degrees of rotation (as long as the chunk of the source bitmap that
            corresponds to an NxN destination block fits in cache!).

            N is defined by ROZ_BLOCK_SIZE below; the best N is one that is as
            big as possible but at the same time not too big to prevent all of
            the source bitmap pixels from fitting into cache at the same time.
            Keep in mind that the block of source pixels used can be somewhat
            scattered in memory.  8x8 works well on the few processors that
            were tested; 16x16 seems to work even better for more modern
            processors with larger caches, but since 8x8 works well enough and
            is less likely to result in cache misses on processors with smaller
            caches, it is used.
        */

#define ROZ_BLOCK_SIZE 8

		UINT32 size_mask = rozInfo->size - 1;
		bitmap_t *srcbitmap = tilemap_get_pixmap(tmap);
		bitmap_t *flagsbitmap = tilemap_get_flagsmap(tmap);
		UINT32 srcx = (rozInfo->startx + (clip->min_x * rozInfo->incxx) +
			(clip->min_y * rozInfo->incyx));
		UINT32 srcy = (rozInfo->starty + (clip->min_x * rozInfo->incxy) +
			(clip->min_y * rozInfo->incyy));
		int destx = clip->min_x;
		int desty = clip->min_y;

		int row_count = (clip->max_y - desty) + 1;
		int row_block_count = row_count / ROZ_BLOCK_SIZE;
		int row_extra_count = row_count % ROZ_BLOCK_SIZE;

		int column_count = (clip->max_x - destx) + 1;
		int column_block_count = column_count / ROZ_BLOCK_SIZE;
		int column_extra_count = column_count % ROZ_BLOCK_SIZE;

		int row_block_size_incxx = ROZ_BLOCK_SIZE * rozInfo->incxx;
		int row_block_size_incxy = ROZ_BLOCK_SIZE * rozInfo->incxy;
		int row_block_size_incyx = ROZ_BLOCK_SIZE * rozInfo->incyx;
		int row_block_size_incyy = ROZ_BLOCK_SIZE * rozInfo->incyy;

		int i,j;

		// Do the block rows
		for (i = 0; i < row_block_count; i++)
		{
			int sx = srcx;
			int sy = srcy;
			int dx = destx;
			// Do the block columns
			for (j = 0; j < column_block_count; j++)
			{
				DrawRozHelperBlock(rozInfo, dx, desty, sx, sy, ROZ_BLOCK_SIZE,
					ROZ_BLOCK_SIZE, bitmap, flagsbitmap, srcbitmap, size_mask);
				// Increment to the next block column
				sx += row_block_size_incxx;
				sy += row_block_size_incxy;
				dx += ROZ_BLOCK_SIZE;
			}
			// Do the extra columns
			if (column_extra_count)
			{
				DrawRozHelperBlock(rozInfo, dx, desty, sx, sy, column_extra_count,
					ROZ_BLOCK_SIZE, bitmap, flagsbitmap, srcbitmap, size_mask);
			}
			// Increment to the next row block
			srcx += row_block_size_incyx;
			srcy += row_block_size_incyy;
			desty += ROZ_BLOCK_SIZE;
		}
		// Do the extra rows
		if (row_extra_count)
		{
			// Do the block columns
			for (i = 0; i < column_block_count; i++)
			{
				DrawRozHelperBlock(rozInfo, destx, desty, srcx, srcy, ROZ_BLOCK_SIZE,
					row_extra_count, bitmap, flagsbitmap, srcbitmap, size_mask);
				srcx += row_block_size_incxx;
				srcy += row_block_size_incxy;
				destx += ROZ_BLOCK_SIZE;
			}
			// Do the extra columns
			if (column_extra_count)
			{
				DrawRozHelperBlock(rozInfo, destx, desty, srcx, srcy, column_extra_count,
					row_extra_count, bitmap, flagsbitmap, srcbitmap, size_mask);
			}
		}
	}
	else
	{
		tilemap_draw_roz(
			bitmap, clip, tmap,
			rozInfo->startx, rozInfo->starty,
			rozInfo->incxx, rozInfo->incxy,
			rozInfo->incyx, rozInfo->incyy,
			rozInfo->wrap,0,0); // wrap, flags, pri
	}
} /* DrawRozHelper */

static void
DrawROZ(bitmap_t *bitmap,const rectangle *cliprect)
{
	const int xoffset = 38,yoffset = 0;
	struct RozParam rozParam;

	rozParam.color = (namcos2_gfx_ctrl & 0x0f00);
	rozParam.incxx  = (INT16)namcos2_68k_roz_ctrl[0];
	rozParam.incxy  = (INT16)namcos2_68k_roz_ctrl[1];
	rozParam.incyx  = (INT16)namcos2_68k_roz_ctrl[2];
	rozParam.incyy  = (INT16)namcos2_68k_roz_ctrl[3];
	rozParam.startx = (INT16)namcos2_68k_roz_ctrl[4];
	rozParam.starty = (INT16)namcos2_68k_roz_ctrl[5];
	rozParam.size = 2048;
	rozParam.wrap = 1;


	switch( namcos2_68k_roz_ctrl[7] )
	{
	case 0x4400: /* (2048x2048) */
		break;

	case 0x4488: /* attract mode */
		rozParam.wrap = 0;
		break;

	case 0x44cc: /* stage1 demo */
		rozParam.wrap = 0;
		break;

	case 0x44ee: /* (256x256) used in Dragon Saber */
		rozParam.wrap = 0;
		rozParam.size = 256;
		break;
	}

	rozParam.startx <<= 4;
	rozParam.starty <<= 4;
	rozParam.startx += xoffset * rozParam.incxx + yoffset * rozParam.incyx;
	rozParam.starty += xoffset * rozParam.incxy + yoffset * rozParam.incyy;

	rozParam.startx<<=8;
	rozParam.starty<<=8;
	rozParam.incxx<<=8;
	rozParam.incxy<<=8;
	rozParam.incyx<<=8;
	rozParam.incyy<<=8;

	DrawRozHelper( bitmap, tilemap_roz, cliprect, &rozParam );
}

READ16_HANDLER(namcos2_68k_roz_ctrl_r)
{
	return namcos2_68k_roz_ctrl[offset];
}

WRITE16_HANDLER( namcos2_68k_roz_ctrl_w )
{
	COMBINE_DATA(&namcos2_68k_roz_ctrl[offset]);
}

READ16_HANDLER( namcos2_68k_roz_ram_r )
{
	return namcos2_68k_roz_ram[offset];
}

WRITE16_HANDLER( namcos2_68k_roz_ram_w )
{
	COMBINE_DATA(&namcos2_68k_roz_ram[offset]);
	tilemap_mark_tile_dirty(tilemap_roz,offset);
//      if( input_code_pressed(space->machine, KEYCODE_Q) )
//      {
//          debugger_break(space->machine);
//      }
}

/**************************************************************************/

static UINT16
GetPaletteRegister( int which )
{
	const UINT16 *source = &namcos2_68k_palette_ram[0x3000/2];
	return ((source[which*2]&0xff)<<8) | (source[which*2+1]&0xff);
}

READ16_HANDLER( namcos2_68k_video_palette_r )
{
	if( (offset&0x1800) == 0x1800 )
	{
		/* palette register */
		offset &= 0x180f;

		/* registers 6,7: unmapped? */
		if (offset > 0x180b) return 0xff;
	}
	return namcos2_68k_palette_ram[offset];
} /* namcos2_68k_video_palette_r */

WRITE16_HANDLER( namcos2_68k_video_palette_w )
{
	if( (offset&0x1800) == 0x1800 )
	{
		/* palette register */
		offset &= 0x180f;

		if( ACCESSING_BITS_0_7 ) data&=0xff;
		else data>>=8;

		switch (offset) {
			/* registers 0-3: clipping */

			/* register 4: ? */
			/* sets using it:
            assault:    $0020
            burnforc:   $0130 after titlescreen
            dirtfoxj:   $0108 at game start
            finalap1/2/3:   $00C0
            finehour:   $0168 after titlescreen
            fourtrax:   $00E8 and $00F0
            luckywld:   $00E8 at titlescreen, $00A0 in game and $0118 if in tunnel
            suzuka8h1/2:    $00E8 and $00A0 */
			case 0x1808: case 0x1809:
				// if (data^namcos2_68k_palette_ram[offset]) printf("%04X\n",data<<((~offset&1)<<3)|namcos2_68k_palette_ram[offset^1]<<((offset&1)<<3));
				break;

			/* register 5: POSIRQ scanline (only 8 bits used) */
			/*case 0x180a:*/ case 0x180b:
				if (data^namcos2_68k_palette_ram[offset]) {
					namcos2_68k_palette_ram[offset] = data;
					namcos2_adjust_posirq_timer(space->machine,namcos2_GetPosIrqScanline(space->machine));
				}
				break;

			/* registers 6,7: nothing? */
			default: break;
		}

		namcos2_68k_palette_ram[offset] = data;
	}
	else
	{
		COMBINE_DATA(&namcos2_68k_palette_ram[offset]);
	}
} /* namcos2_68k_video_palette_w */


int
namcos2_GetPosIrqScanline( running_machine *machine )
{
	return (GetPaletteRegister(5) - 32) & 0xff;
} /* namcos2_GetPosIrqScanline */

static void
UpdatePalette( running_machine *machine )
{
	int bank;
	for( bank=0; bank<0x20; bank++ )
	{
		int pen = bank*256;
		int offset = ((pen & 0x1800) << 2) | (pen & 0x07ff);
		int i;
		for( i=0; i<256; i++ )
		{
			int r = namcos2_68k_palette_ram[offset | 0x0000] & 0x00ff;
			int g = namcos2_68k_palette_ram[offset | 0x0800] & 0x00ff;
			int b = namcos2_68k_palette_ram[offset | 0x1000] & 0x00ff;
			palette_set_color(machine,pen++,MAKE_RGB(r,g,b));
			offset++;
		}
	}
} /* UpdatePalette */

/**************************************************************************/

static void
DrawSpriteInit( running_machine *machine )
{
	int i;
	/* set table for sprite color == 0x0f */
	for( i = 0; i<16*256; i++ )
	{
		machine->shadow_table[i] = i+0x2000;
	}
}

WRITE16_HANDLER( namcos2_sprite_ram_w )
{
	COMBINE_DATA(&namcos2_sprite_ram[offset]);
}

READ16_HANDLER( namcos2_sprite_ram_r )
{
	return namcos2_sprite_ram[offset];
}

/**************************************************************************/

VIDEO_START( namcos2 )
{
	namco_tilemap_init(machine,2,memory_region(machine, "gfx4"),TilemapCB);
	tilemap_roz = tilemap_create(machine, get_tile_info_roz,tilemap_scan_rows,8,8,256,256);
	tilemap_set_transparent_pen(tilemap_roz,0xff);
	DrawSpriteInit(machine);
}

static void
ApplyClip( rectangle *clip, const rectangle *cliprect )
{
	clip->min_x = GetPaletteRegister(0) - 0x4a;
	clip->max_x = GetPaletteRegister(1) - 0x4a - 1;
	clip->min_y = GetPaletteRegister(2) - 0x21;
	clip->max_y = GetPaletteRegister(3) - 0x21 - 1;
	/* intersect with master clip rectangle */
	sect_rect(clip, cliprect);
} /* ApplyClip */

VIDEO_UPDATE( namcos2_default )
{
	rectangle clip;
	int pri;

	UpdatePalette(screen->machine);
	bitmap_fill( bitmap, cliprect , get_black_pen(screen->machine));
	ApplyClip( &clip, cliprect );

	/* HACK: enable ROZ layer only if it has priority > 0 */
	tilemap_set_enable(tilemap_roz,(namcos2_gfx_ctrl & 0x7000) ? 1 : 0);

	for( pri=0; pri<16; pri++ )
	{
		if( (pri&1)==0 )
		{
			namco_tilemap_draw( bitmap, &clip, pri/2 );

			if( ((namcos2_gfx_ctrl & 0x7000) >> 12)==pri/2 )
			{
				DrawROZ(bitmap,&clip);
			}
			namcos2_draw_sprites(screen->machine, bitmap, &clip, pri/2, namcos2_gfx_ctrl );
		}
	}
	return 0;
}

/**************************************************************************/

VIDEO_START( finallap )
{
	namco_tilemap_init(machine,2,memory_region(machine, "gfx4"),TilemapCB);
	DrawSpriteInit(machine);
	namco_road_init(machine, 3);
}

VIDEO_UPDATE( finallap )
{
	rectangle clip;
	int pri;

	UpdatePalette(screen->machine);
	bitmap_fill( bitmap, cliprect , get_black_pen(screen->machine));
	ApplyClip( &clip, cliprect );

	for( pri=0; pri<16; pri++ )
	{
		if( (pri&1)==0 )
		{
			namco_tilemap_draw( bitmap, &clip, pri/2 );
		}
		namco_road_draw(screen->machine, bitmap,&clip,pri );
		namcos2_draw_sprites(screen->machine, bitmap,&clip,pri,namcos2_gfx_ctrl );
	}
	return 0;
}

/**************************************************************************/

VIDEO_START( luckywld )
{
	namco_tilemap_init(machine,2,memory_region(machine, "gfx4"),TilemapCB);
	namco_obj_init( machine, 0, 0x0, NULL );
	if( namcos2_gametype==NAMCOS2_LUCKY_AND_WILD )
	{
		namco_roz_init( machine, 1, "gfx5" );
	}
	if( namcos2_gametype!=NAMCOS2_STEEL_GUNNER_2 )
	{
		namco_road_init(machine, 3);
	}
} /* luckywld */

VIDEO_UPDATE( luckywld )
{
	rectangle clip;
	int pri;

	UpdatePalette(screen->machine);
	bitmap_fill( bitmap, cliprect , get_black_pen(screen->machine));
	ApplyClip( &clip, cliprect );

	for( pri=0; pri<16; pri++ )
	{
		if( (pri&1)==0 )
		{
			namco_tilemap_draw( bitmap, &clip, pri/2 );
		}
		namco_road_draw(screen->machine, bitmap,&clip,pri );
		if( namcos2_gametype==NAMCOS2_LUCKY_AND_WILD )
		{
			namco_roz_draw( bitmap, &clip, pri );
		}
		namco_obj_draw(screen->machine, bitmap, &clip, pri );
	}
	return 0;
}

/**************************************************************************/

VIDEO_START( sgunner )
{
	namco_tilemap_init(machine,2,memory_region(machine, "gfx4"),TilemapCB);
	namco_obj_init( machine, 0, 0x0, NULL );
}

VIDEO_UPDATE( sgunner )
{
	rectangle clip;
	int pri;

	UpdatePalette(screen->machine);
	bitmap_fill( bitmap, cliprect , get_black_pen(screen->machine));
	ApplyClip( &clip, cliprect );

	for( pri=0; pri<8; pri++ )
	{
		namco_tilemap_draw( bitmap, &clip, pri );
		namco_obj_draw(screen->machine, bitmap, &clip, pri );
	}
	return 0;
}


/**************************************************************************/

VIDEO_START( metlhawk )
{
	namco_tilemap_init(machine,2,memory_region(machine, "gfx4"),TilemapCB);
	namco_roz_init( machine, 1, "gfx5" );
}

VIDEO_UPDATE( metlhawk )
{
	rectangle clip;
	int pri;

	UpdatePalette(screen->machine);
	bitmap_fill( bitmap, cliprect , get_black_pen(screen->machine));
	ApplyClip( &clip, cliprect );

	for( pri=0; pri<16; pri++ )
	{
		if( (pri&1)==0 )
		{
			namco_tilemap_draw( bitmap, &clip, pri/2 );
		}
		namco_roz_draw( bitmap, &clip, pri );
		namcos2_draw_sprites_metalhawk(screen->machine, bitmap,&clip,pri );
	}
	return 0;
}
