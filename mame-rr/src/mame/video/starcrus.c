
/* Ramtek - Star Cruiser */

#include "emu.h"
#include "sound/samples.h"

static bitmap_t *ship1_vid;
static bitmap_t *ship2_vid;
static bitmap_t *proj1_vid;
static bitmap_t *proj2_vid;

static int s1_x = 0;
static int s1_y = 0;
static int s2_x = 0;
static int s2_y = 0;
static int p1_x = 0;
static int p1_y = 0;
static int p2_x = 0;
static int p2_y = 0;

static int p1_sprite = 0;
static int p2_sprite = 0;
static int s1_sprite = 0;
static int s2_sprite = 0;

static int engine1_on = 0;
static int engine2_on = 0;
static int explode1_on = 0;
static int explode2_on = 0;
static int launch1_on = 0;
static int launch2_on = 0;

/* The collision detection techniques use in this driver
   are well explained in the comments in the sprint2 driver */

static int collision_reg = 0x00;

/* I hate to have sound in video, but the sprite and
   audio bits are in the same bytes, and there are so few
   samples... */

static int starcrus_engine_sound_playing = 0;
static int starcrus_explode_sound_playing = 0;
static int starcrus_launch1_sound_playing = 0;
static int starcrus_launch2_sound_playing = 0;

WRITE8_HANDLER( starcrus_s1_x_w ) { s1_x = data^0xff; }
WRITE8_HANDLER( starcrus_s1_y_w ) { s1_y = data^0xff; }
WRITE8_HANDLER( starcrus_s2_x_w ) { s2_x = data^0xff; }
WRITE8_HANDLER( starcrus_s2_y_w ) { s2_y = data^0xff; }
WRITE8_HANDLER( starcrus_p1_x_w ) { p1_x = data^0xff; }
WRITE8_HANDLER( starcrus_p1_y_w ) { p1_y = data^0xff; }
WRITE8_HANDLER( starcrus_p2_x_w ) { p2_x = data^0xff; }
WRITE8_HANDLER( starcrus_p2_y_w ) { p2_y = data^0xff; }

VIDEO_START( starcrus )
{
	ship1_vid = auto_bitmap_alloc(machine,16,16,machine->primary_screen->format());
	ship2_vid = auto_bitmap_alloc(machine,16,16,machine->primary_screen->format());

	proj1_vid = auto_bitmap_alloc(machine,16,16,machine->primary_screen->format());
	proj2_vid = auto_bitmap_alloc(machine,16,16,machine->primary_screen->format());
}

WRITE8_HANDLER( starcrus_ship_parm_1_w )
{
	running_device *samples = space->machine->device("samples");

    s1_sprite = data&0x1f;
    engine1_on = ((data&0x20)>>5)^0x01;

    if (engine1_on || engine2_on)
    {
		if (starcrus_engine_sound_playing == 0)
		{
        	starcrus_engine_sound_playing = 1;
        	sample_start(samples, 0,0,1);	/* engine sample */

		}
    }
    else
    {
		if (starcrus_engine_sound_playing == 1)
		{
        	starcrus_engine_sound_playing = 0;
			sample_stop(samples, 0);
		}
	}
}

WRITE8_HANDLER( starcrus_ship_parm_2_w )
{
	running_device *samples = space->machine->device("samples");

    s2_sprite = data&0x1f;
    set_led_status(space->machine, 2,~data & 0x80); 		/* game over lamp */
    coin_counter_w(space->machine, 0, ((data&0x40)>>6)^0x01);	/* coin counter */
    engine2_on = ((data&0x20)>>5)^0x01;

    if (engine1_on || engine2_on)
    {
		if (starcrus_engine_sound_playing == 0)
		{
        	starcrus_engine_sound_playing = 1;
        	sample_start(samples,0,0,1);	/* engine sample */
		}
    }
    else
    {
		if (starcrus_engine_sound_playing == 1)
		{
        	starcrus_engine_sound_playing = 0;
			sample_stop(samples,0);
		}
	}

}

WRITE8_HANDLER( starcrus_proj_parm_1_w )
{
	running_device *samples = space->machine->device("samples");

    p1_sprite = data&0x0f;
    launch1_on = ((data&0x20)>>5)^0x01;
    explode1_on = ((data&0x10)>>4)^0x01;

    if (explode1_on || explode2_on)
    {
		if (starcrus_explode_sound_playing == 0)
		{
			starcrus_explode_sound_playing = 1;
			sample_start(samples, 1,1,1);	/* explosion initial sample */
		}
	}
	else
    {
		if (starcrus_explode_sound_playing == 1)
		{
			starcrus_explode_sound_playing = 0;
			sample_start(samples, 1,2,0);	/* explosion ending sample */
		}
	}

	if (launch1_on)
	{
		if (starcrus_launch1_sound_playing == 0)
		{
			starcrus_launch1_sound_playing = 1;
			sample_start(samples, 2,3,0);	/* launch sample */
		}
	}
	else
	{
		starcrus_launch1_sound_playing = 0;
	}
}

WRITE8_HANDLER( starcrus_proj_parm_2_w )
{
	running_device *samples = space->machine->device("samples");

    p2_sprite = data&0x0f;
    launch2_on = ((data&0x20)>>5)^0x01;
    explode2_on = ((data&0x10)>>4)^0x01;

    if (explode1_on || explode2_on)
    {
		if (starcrus_explode_sound_playing == 0)
		{
			starcrus_explode_sound_playing = 1;
			sample_start(samples, 1,1,1);	/* explosion initial sample */
		}
	}
	else
    {
		if (starcrus_explode_sound_playing == 1)
		{
			starcrus_explode_sound_playing = 0;
			sample_start(samples, 1,2,0);	/* explosion ending sample */
		}
	}

	if (launch2_on)
	{
		if (starcrus_launch2_sound_playing == 0)
		{
			starcrus_launch2_sound_playing = 1;
			sample_start(samples, 3,3,0);	/* launch sample */
		}
	}
	else
	{
		starcrus_launch2_sound_playing = 0;
	}
}

static int collision_check_s1s2(running_machine *machine)
{
	int org_x, org_y;
	int sx, sy;
	rectangle clip;

    clip.min_x=0;
    clip.max_x=15;
    clip.min_y=0;
    clip.max_y=15;

    bitmap_fill(ship1_vid,&clip,0);
    bitmap_fill(ship2_vid,&clip,0);

	/* origin is with respect to ship1 */

	org_x = s1_x;
	org_y = s1_y;

	/* Draw ship 1 */
    drawgfx_opaque(ship1_vid,
            &clip,
            machine->gfx[8+((s1_sprite&0x04)>>2)],
            (s1_sprite&0x03)^0x03,
            0,
            (s1_sprite&0x08)>>3,(s1_sprite&0x10)>>4,
            s1_x-org_x,s1_y-org_y);

	/* Draw ship 2 */
    drawgfx_opaque(ship2_vid,
            &clip,
            machine->gfx[10+((s2_sprite&0x04)>>2)],
            (s2_sprite&0x03)^0x03,
            0,
            (s2_sprite&0x08)>>3,(s2_sprite&0x10)>>4,
            s2_x-org_x,s2_y-org_y);

    /* Now check for collisions */
    for (sy=0;sy<16;sy++)
        for (sx=0;sx<16;sx++)
    		/* Condition 1 - ship 1 = ship 2 */
        	if ((*BITMAP_ADDR16(ship1_vid, sy, sx)==1) && (*BITMAP_ADDR16(ship2_vid, sy, sx)==1))
                return 1;

    return 0;
}

static int collision_check_p1p2(running_machine *machine)
{
	int org_x, org_y;
	int sx, sy;
	rectangle clip;

	/* if both are scores, return */
	if ( ((p1_sprite & 0x08) == 0) &&
         ((p2_sprite & 0x08) == 0) )
	{
		return 0;
	}

    clip.min_x=0;
    clip.max_x=15;
    clip.min_y=0;
    clip.max_y=15;

    bitmap_fill(proj1_vid,&clip,0);
    bitmap_fill(proj2_vid,&clip,0);

	/* origin is with respect to proj1 */

	org_x = p1_x;
	org_y = p1_y;

	if (p1_sprite & 0x08)	/* if p1 is a projectile */
	{
		/* Draw score/projectile 1 */
		drawgfx_opaque(proj1_vid,
				&clip,
				machine->gfx[(p1_sprite&0x0c)>>2],
				(p1_sprite&0x03)^0x03,
				0,
				0,0,
				p1_x-org_x,p1_y-org_y);
	}

	if (p2_sprite & 0x08)	/* if p2 is a projectile */
	{
		/* Draw score/projectile 2 */
		drawgfx_opaque(proj2_vid,
				&clip,
				machine->gfx[4+((p2_sprite&0x0c)>>2)],
				(p2_sprite&0x03)^0x03,
				0,
				0,0,
				p2_x-org_x,p2_y-org_y);
	}

    /* Now check for collisions */
    for (sy=0;sy<16;sy++)
        for (sx=0;sx<16;sx++)
    		/* Condition 1 - proj 1 = proj 2 */
        	if ((*BITMAP_ADDR16(proj1_vid, sy, sx)==1) && (*BITMAP_ADDR16(proj2_vid, sy, sx)==1))
                return 1;

    return 0;
}

static int collision_check_s1p1p2(running_machine *machine)
{
	int org_x, org_y;
	int sx, sy;
	rectangle clip;

	/* if both are scores, return */
	if ( ((p1_sprite & 0x08) == 0) &&
         ((p2_sprite & 0x08) == 0) )
	{
		return 0;
	}

    clip.min_x=0;
    clip.max_x=15;
    clip.min_y=0;
    clip.max_y=15;

    bitmap_fill(ship1_vid,&clip,0);
    bitmap_fill(proj1_vid,&clip,0);
    bitmap_fill(proj2_vid,&clip,0);

	/* origin is with respect to ship1 */

	org_x = s1_x;
	org_y = s1_y;

	/* Draw ship 1 */
    drawgfx_opaque(ship1_vid,
            &clip,
            machine->gfx[8+((s1_sprite&0x04)>>2)],
            (s1_sprite&0x03)^0x03,
            0,
            (s1_sprite&0x08)>>3,(s1_sprite&0x10)>>4,
            s1_x-org_x,s1_y-org_y);

	if (p1_sprite & 0x08)	/* if p1 is a projectile */
	{
		/* Draw projectile 1 */
		drawgfx_opaque(proj1_vid,
				&clip,
				machine->gfx[(p1_sprite&0x0c)>>2],
				(p1_sprite&0x03)^0x03,
				0,
				0,0,
				p1_x-org_x,p1_y-org_y);
	}

	if (p2_sprite & 0x08)	/* if p2 is a projectile */
	{
		/* Draw projectile 2 */
		drawgfx_opaque(proj2_vid,
				&clip,
				machine->gfx[4+((p2_sprite&0x0c)>>2)],
				(p2_sprite&0x03)^0x03,
				0,
				0,0,
				p2_x-org_x,p2_y-org_y);
	}

    /* Now check for collisions */
    for (sy=0;sy<16;sy++)
        for (sx=0;sx<16;sx++)
        	if (*BITMAP_ADDR16(ship1_vid, sy, sx)==1)
        	{
        		/* Condition 1 - ship 1 = proj 1 */
				if (*BITMAP_ADDR16(proj1_vid, sy, sx)==1)
                	return 1;
        		/* Condition 2 - ship 1 = proj 2 */
        		if (*BITMAP_ADDR16(proj2_vid, sy, sx)==1)
                	return 1;
            }

    return 0;
}

static int collision_check_s2p1p2(running_machine *machine)
{
	int org_x, org_y;
	int sx, sy;
	rectangle clip;

	/* if both are scores, return */
	if ( ((p1_sprite & 0x08) == 0) &&
         ((p2_sprite & 0x08) == 0) )
	{
		return 0;
	}

    clip.min_x=0;
    clip.max_x=15;
    clip.min_y=0;
    clip.max_y=15;

    bitmap_fill(ship2_vid,&clip,0);
    bitmap_fill(proj1_vid,&clip,0);
    bitmap_fill(proj2_vid,&clip,0);

	/* origin is with respect to ship2 */

	org_x = s2_x;
	org_y = s2_y;

	/* Draw ship 2 */
    drawgfx_opaque(ship2_vid,
            &clip,
            machine->gfx[10+((s2_sprite&0x04)>>2)],
            (s2_sprite&0x03)^0x03,
            0,
            (s2_sprite&0x08)>>3,(s2_sprite&0x10)>>4,
            s2_x-org_x,s2_y-org_y);

	if (p1_sprite & 0x08)	/* if p1 is a projectile */
	{
		/* Draw projectile 1 */
		drawgfx_opaque(proj1_vid,
				&clip,
				machine->gfx[(p1_sprite&0x0c)>>2],
				(p1_sprite&0x03)^0x03,
				0,
				0,0,
				p1_x-org_x,p1_y-org_y);
	}

	if (p2_sprite & 0x08)	/* if p2 is a projectile */
	{
		/* Draw projectile 2 */
		drawgfx_opaque(proj2_vid,
				&clip,
				machine->gfx[4+((p2_sprite&0x0c)>>2)],
				(p2_sprite&0x03)^0x03,
				0,
				0,0,
				p2_x-org_x,p2_y-org_y);
	}

    /* Now check for collisions */
    for (sy=0;sy<16;sy++)
        for (sx=0;sx<16;sx++)
        	if (*BITMAP_ADDR16(ship2_vid, sy, sx)==1)
        	{
        		/* Condition 1 - ship 2 = proj 1 */
				if (*BITMAP_ADDR16(proj1_vid, sy, sx)==1)
                	return 1;
        		/* Condition 2 - ship 2 = proj 2 */
        		if (*BITMAP_ADDR16(proj2_vid, sy, sx)==1)
                	return 1;
            }

    return 0;
}

VIDEO_UPDATE( starcrus )
{
    bitmap_fill(bitmap,cliprect,0);

	/* Draw ship 1 */
    drawgfx_transpen(bitmap,
            cliprect,
            screen->machine->gfx[8+((s1_sprite&0x04)>>2)],
            (s1_sprite&0x03)^0x03,
            0,
            (s1_sprite&0x08)>>3,(s1_sprite&0x10)>>4,
            s1_x,s1_y,
            0);

	/* Draw ship 2 */
    drawgfx_transpen(bitmap,
            cliprect,
            screen->machine->gfx[10+((s2_sprite&0x04)>>2)],
            (s2_sprite&0x03)^0x03,
            0,
            (s2_sprite&0x08)>>3,(s2_sprite&0x10)>>4,
            s2_x,s2_y,
            0);

	/* Draw score/projectile 1 */
	drawgfx_transpen(bitmap,
            cliprect,
            screen->machine->gfx[(p1_sprite&0x0c)>>2],
            (p1_sprite&0x03)^0x03,
            0,
            0,0,
            p1_x,p1_y,
            0);

	/* Draw score/projectile 2 */
	drawgfx_transpen(bitmap,
            cliprect,
            screen->machine->gfx[4+((p2_sprite&0x0c)>>2)],
            (p2_sprite&0x03)^0x03,
            0,
            0,0,
            p2_x,p2_y,
            0);

    /* Collision detection */
	if (cliprect->max_y == screen->visible_area().max_y)
	{
		collision_reg = 0x00;

		/* Check for collisions between ship1 and ship2 */
		if (collision_check_s1s2(screen->machine))
		{
			collision_reg |= 0x08;
		}
		/* Check for collisions between ship1 and projectiles */
		if (collision_check_s1p1p2(screen->machine))
		{
			collision_reg |= 0x02;
		}
		/* Check for collisions between ship1 and projectiles */
		if (collision_check_s2p1p2(screen->machine))
		{
			collision_reg |= 0x01;
		}
		/* Check for collisions between ship1 and projectiles */
		/* Note: I don't think this is used by the game */
		if (collision_check_p1p2(screen->machine))
		{
			collision_reg |= 0x04;
		}
	}

	return 0;
}

READ8_HANDLER( starcrus_coll_det_r )
{
    return collision_reg ^ 0xff;
}
