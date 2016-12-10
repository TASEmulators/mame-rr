/* video/shangkid */

#include "emu.h"
#include "cpu/z80/z80.h"

static tilemap_t *background;
UINT8 *shangkid_videoreg;
int shangkid_gfx_type;



static TILE_GET_INFO( get_bg_tile_info ){
	int attributes = machine->generic.videoram.u8[tile_index+0x800];
	int tile_number = machine->generic.videoram.u8[tile_index]+0x100*(attributes&0x3);
	int color;

	if( shangkid_gfx_type==1 )
	{
		/* Shanghai Kid:
            ------xx    bank
            -----x--    flipx
            xxxxx---    color
        */
		color = attributes>>3;
		color = (color&0x03)|((color&0x1c)<<1);
		SET_TILE_INFO(
				0,
				tile_number,
				color,
				(attributes&0x04)?TILE_FLIPX:0);
	}
	else
	{
		/* Chinese Hero:
            ------xx    bank
            -xxxxx--    color
            x-------    flipx?
        */
		color = (attributes>>2)&0x1f;
		SET_TILE_INFO(
				0,
				tile_number,
				color,
				(attributes&0x80)?TILE_FLIPX:0);
	}

	tileinfo->category =
		(memory_region( machine, "proms" )[0x800+color*4]==2)?1:0;
}

VIDEO_START( shangkid )
{
	background = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,8,8,64,32);
}

WRITE8_HANDLER( shangkid_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty( background, offset&0x7ff );
}

static void draw_sprite(running_machine *machine, const UINT8 *source, bitmap_t *bitmap, const rectangle *cliprect ){
	const gfx_element *gfx;
	int transparent_pen;
	int bank_index;
	int c,r;
	int width,height;
	int sx,sy;

	int ypos		= 209 - source[0];
	int tile		= source[1]&0x3f;
	int xflip		= (source[1]&0x40)?1:0;
	int yflip		= (source[1]&0x80)?1:0;
	int bank		= source[2]&0x3f;
	int xsize		= (source[2]&0x40)?1:0;
	int ysize		= (source[2]&0x80)?1:0;
	int yscale		= source[3]&0x07;	/* 0x0 = smallest; 0x7 = biggest */
	int xpos		= ((source[4]+source[5]*255)&0x1ff)-23;
	int color		= source[6]&0x3f;
	int xscale		= source[7]&0x07;	/* 0x0 = smallest; 0x7 = biggest */

	/* adjust placement for small sprites */
	if( xsize==0 && xflip ) xpos -= 16;
	if( ysize==0 && yflip==0 ) ypos += 16;

	if( shangkid_gfx_type == 1 )
	{
		/* Shanghai Kid */
		switch( bank&0x30 )
		{
		case 0x00:
		case 0x10:
			tile += 0x40*(bank&0xf);
			break;

		case 0x20:
			tile += 0x40*((bank&0x3)|0x10);
			break;

		case 0x30:
			tile += 0x40*((bank&0x3)|0x14);
			break;
		}
		bank_index = 0;
		transparent_pen = 3;
	}
	else
	{
		/* Chinese Hero */
		color >>= 1;
		switch( bank>>2 )
		{
		case 0x0: bank_index = 0; break;
		case 0x9: bank_index = 1; break;
		case 0x6: bank_index = 2; break;
		case 0xf: bank_index = 3; break;
		default:
			bank_index = 0;
			break;
		}

		if( bank&0x01 ) tile += 0x40;
		transparent_pen = 7;
	}

	gfx = machine->gfx[1+bank_index];

	width = (xscale+1)*2;
	height = (yscale+1)*2;

	/* center zoomed sprites */
	xpos += (16-width)*(xsize+1)/2;
	ypos += (16-height)*(ysize+1)/2;

	for( r=0; r<=ysize; r++ )
	{
		for( c=0; c<=xsize; c++ )
		{
			sx = xpos+(c^xflip)*width;
			sy = ypos+(r^yflip)*height;
			drawgfxzoom_transpen(
				bitmap,
				cliprect,
				gfx,
				tile+c*8+r,
				color,
				xflip,yflip,
				sx,sy,
				(width<<16)/16, (height<<16)/16,transparent_pen );
		}
	}
}

static void shangkid_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	const UINT8 *source, *finish;

	finish = machine->generic.spriteram.u8;
	source = machine->generic.spriteram.u8+0x200;
	while( source>finish ){
		source -= 8;
		draw_sprite(machine, source, bitmap,cliprect );
	}
}

VIDEO_UPDATE( shangkid )
{
	int flipscreen = shangkid_videoreg[1]&0x80;
	tilemap_set_flip( background, flipscreen?(TILEMAP_FLIPX|TILEMAP_FLIPY):0 );
	tilemap_set_scrollx( background,0,shangkid_videoreg[0]-40 );
	tilemap_set_scrolly( background,0,shangkid_videoreg[2]+0x10 );

	tilemap_draw( bitmap,cliprect,background,0,0 );
	shangkid_draw_sprites(screen->machine, bitmap,cliprect );
	tilemap_draw( bitmap,cliprect,background,1,0 ); /* high priority tiles */
	return 0;
}


PALETTE_INIT( dynamski )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		UINT16 data = (color_prom[i | 0x20] << 8) | color_prom[i];
		rgb_t color = MAKE_RGB(pal5bit(data >> 1), pal5bit(data >> 6), pal5bit(data >> 11));

		colortable_palette_set_color(machine->colortable, i, color);
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x40;

	/* characters */
	for (i = 0; i < 0x40; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* sprites */
	for (i = 0x40; i < 0x80; i++)
	{
		UINT8 ctabentry = (color_prom[(i - 0x40) + 0x100] & 0x0f) | 0x10;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}


static void dynamski_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int pri )
{
	int i;
	int sx,sy;
	int tile;
	int attr;
	int temp;

	for( i=0; i<0x400; i++ )
	{
		sx = (i%32)*8;
		sy = (i/32)*8;

		if( sy<16 )
		{
			temp = sx;
			sx = sy+256+16;
			sy = temp;
		}
		else if( sy>=256-16 )
		{
			temp = sx;
			sx = sy-256+16;
			sy = temp;
		}
		else
		{
			sx+=16;
		}

		tile = machine->generic.videoram.u8[i];
		attr = machine->generic.videoram.u8[i+0x400];
		/*
            x---.----   priority?
            -xx-.----   bank
        */
		if( pri==0 || (attr>>7)==pri )
		{
			tile += ((attr>>5)&0x3)*256;
			drawgfx_transpen(
				bitmap,
				cliprect,
				machine->gfx[0],
				tile,
				attr & 0x0f,
				0,0,//xflip,yflip,
				sx,sy,
				pri?3:-1 );
		}
	}
}

static void dynamski_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	int i;
	int sx,sy;
	int tile;
	int bank;
	int attr;
	int color;
	for( i=0x7e; i>=0x00; i-=2 )
	{
		bank = machine->generic.videoram.u8[0x1b80+i];
		attr = machine->generic.videoram.u8[0x1b81+i];
		tile = machine->generic.videoram.u8[0xb80+i];
		color = machine->generic.videoram.u8[0xb81+i];
		sy = 240-machine->generic.videoram.u8[0x1380+i];

		sx = machine->generic.videoram.u8[0x1381+i]-64+8+16;
		if( attr&1 ) sx += 0x100;

		drawgfx_transpen(
				bitmap,
				cliprect,
				machine->gfx[1],
				bank*0x40 + (tile&0x3f),
				color,
				tile&0x80,tile&0x40, /* flipx,flipy */
				sx,sy,3 );
	}
}

VIDEO_UPDATE( dynamski )
{
	dynamski_draw_background(screen->machine, bitmap,cliprect, 0 );
	dynamski_draw_sprites(screen->machine, bitmap,cliprect );
	dynamski_draw_background(screen->machine, bitmap,cliprect, 1 );
	return 0;
}
