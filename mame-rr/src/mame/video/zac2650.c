/*************************************************************/
/*                                                           */
/* Zaccaria/Zelco S2650 based games video                    */
/*                                                           */
/*************************************************************/

#include "emu.h"

UINT8 *zac2650_s2636_0_ram;
static bitmap_t *spritebitmap;

static int CollisionBackground;
static int CollisionSprite;

static tilemap_t *bg_tilemap;


/**************************************************************/
/* The S2636 is a standard sprite chip used by several boards */
/* Emulation of this chip may be moved into a seperate unit   */
/* once it's workings are fully understood.                   */
/**************************************************************/

WRITE8_HANDLER( tinvader_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

READ8_HANDLER( zac_s2636_r )
{
	if(offset!=0xCB) return zac2650_s2636_0_ram[offset];
    else return CollisionSprite;
}

WRITE8_HANDLER( zac_s2636_w )
{
	zac2650_s2636_0_ram[offset] = data;
	gfx_element_mark_dirty(space->machine->gfx[1], offset/8);
	gfx_element_mark_dirty(space->machine->gfx[2], offset/8);
}

READ8_HANDLER( tinvader_port_0_r )
{
	return input_port_read(space->machine, "1E80") - CollisionBackground;
}

/*****************************************/
/* Check for Collision between 2 sprites */
/*****************************************/

static int SpriteCollision(running_machine *machine, int first,int second)
{
	int Checksum=0;
	int x,y;
	const rectangle &visarea = machine->primary_screen->visible_area();

    if((zac2650_s2636_0_ram[first * 0x10 + 10] < 0xf0) && (zac2650_s2636_0_ram[second * 0x10 + 10] < 0xf0))
    {
    	int fx     = (zac2650_s2636_0_ram[first * 0x10 + 10] * 4)-22;
        int fy     = (zac2650_s2636_0_ram[first * 0x10 + 12] * 3)+3;
		int expand = (first==1) ? 2 : 1;

        /* Draw first sprite */

	    drawgfx_opaque(spritebitmap,0, machine->gfx[expand],
			    first * 2,
			    0,
			    0,0,
			    fx,fy);

        /* Get fingerprint */

	    for (x = fx; x < fx + machine->gfx[expand]->width; x++)
	    {
		    for (y = fy; y < fy + machine->gfx[expand]->height; y++)
            {
			    if ((x < visarea.min_x) ||
			        (x > visarea.max_x) ||
			        (y < visarea.min_y) ||
			        (y > visarea.max_y))
			    {
				    continue;
			    }

        	    Checksum += *BITMAP_ADDR16(spritebitmap, y, x);
            }
	    }

        /* Blackout second sprite */

	    drawgfx_transpen(spritebitmap,0, machine->gfx[1],
			    second * 2,
			    1,
			    0,0,
			    (zac2650_s2636_0_ram[second * 0x10 + 10] * 4)-22,(zac2650_s2636_0_ram[second * 0x10 + 12] * 3) + 3, 0);

        /* Remove fingerprint */

	    for (x = fx; x < fx + machine->gfx[expand]->width; x++)
	    {
		    for (y = fy; y < fy + machine->gfx[expand]->height; y++)
            {
			    if ((x < visarea.min_x) ||
			        (x > visarea.max_x) ||
			        (y < visarea.min_y) ||
			        (y > visarea.max_y))
			    {
				    continue;
			    }

        	    Checksum -= *BITMAP_ADDR16(spritebitmap, y, x);
            }
	    }

        /* Zero bitmap */

	    drawgfx_opaque(spritebitmap,0, machine->gfx[expand],
			    first * 2,
			    1,
			    0,0,
			    fx,fy);
    }

	return Checksum;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = machine->generic.videoram.u8[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}

VIDEO_START( tinvader )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 24, 24, 32, 32);

	spritebitmap = machine->primary_screen->alloc_compatible_bitmap();
	machine->generic.tmpbitmap = machine->primary_screen->alloc_compatible_bitmap();

	gfx_element_set_source(machine->gfx[1], zac2650_s2636_0_ram);
	gfx_element_set_source(machine->gfx[2], zac2650_s2636_0_ram);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap)
{
	int offs;
	const rectangle &visarea = machine->primary_screen->visible_area();

    /* -------------------------------------------------------------- */
    /* There seems to be a strange setup with this board, in that it  */
    /* appears that the S2636 runs from a different clock than the    */
    /* background generator, When the program maps sprite position to */
    /* character position it only has 6 pixels of sprite for 8 pixels */
    /* of character.                                                  */
    /* -------------------------------------------------------------- */
    /* n.b. The original has several graphic glitches as well, so it  */
    /* does not seem to be a fault of the emulation!                  */
    /* -------------------------------------------------------------- */

    CollisionBackground = 0;	/* Read from 0x1e80 bit 7 */

	// for collision detection checking
	copybitmap(machine->generic.tmpbitmap,bitmap,0,0,0,0,&visarea);

    for(offs=0;offs<0x50;offs+=0x10)
    {
    	if((zac2650_s2636_0_ram[offs+10]<0xF0) && (offs!=0x30))
		{
            int spriteno = (offs / 8);
			int expand   = ((zac2650_s2636_0_ram[0xc0] & (spriteno*2))!=0) ? 2 : 1;
            int bx       = (zac2650_s2636_0_ram[offs+10] * 4) - 22;
            int by       = (zac2650_s2636_0_ram[offs+12] * 3) + 3;
            int x,y;

            /* Sprite->Background collision detection */
			drawgfx_transpen(bitmap,0, machine->gfx[expand],
				    spriteno,
					1,
				    0,0,
				    bx,by, 0);

	        for (x = bx; x < bx + machine->gfx[expand]->width; x++)
	        {
		        for (y = by; y < by + machine->gfx[expand]->height; y++)
                {
			        if ((x < visarea.min_x) ||
			            (x > visarea.max_x) ||
			            (y < visarea.min_y) ||
			            (y > visarea.max_y))
			        {
				        continue;
			        }

        	        if (*BITMAP_ADDR16(bitmap, y, x) != *BITMAP_ADDR16(machine->generic.tmpbitmap, y, x))
        	        {
                    	CollisionBackground = 0x80;
				        break;
			        }
                }
	        }

			drawgfx_transpen(bitmap,0, machine->gfx[expand],
				    spriteno,
					0,
				    0,0,
				    bx,by, 0);
        }
    }

    /* Sprite->Sprite collision detection */
    CollisionSprite = 0;
//  if(SpriteCollision(machine, 0,1)) CollisionSprite |= 0x20;   /* Not Used */
    if(SpriteCollision(machine, 0,2)) CollisionSprite |= 0x10;
    if(SpriteCollision(machine, 0,4)) CollisionSprite |= 0x08;
    if(SpriteCollision(machine, 1,2)) CollisionSprite |= 0x04;
    if(SpriteCollision(machine, 1,4)) CollisionSprite |= 0x02;
//  if(SpriteCollision(machine, 2,4)) CollisionSprite |= 0x01;   /* Not Used */
}

VIDEO_UPDATE( tinvader )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap);
	return 0;
}
