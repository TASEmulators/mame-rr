/***************************************************************************

GAME PLAN driver

driver by Chris Moore

****************************************************************************/

#include "emu.h"
#include "machine/6522via.h"
#include "includes/gameplan.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define HTOTAL              (0x160)
#define HBEND               (0x000)
#define HBSTART             (0x100)
#define VTOTAL              (0x118)
#define VBEND               (0x000)
#define VBSTART             (0x100)

#define GAMEPLAN_NUM_PENS   (0x08)
#define LEPRECHN_NUM_PENS   (0x10)



/*************************************
 *
 *  Palette handling
 *
 *************************************/

static void gameplan_get_pens( pen_t *pens )
{
	offs_t i;

	for (i = 0; i < GAMEPLAN_NUM_PENS; i++)
		pens[i] = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
}


/* RGBI palette. Is it correct, or does it use the standard RGB? */
static void leprechn_get_pens( pen_t *pens )
{
	offs_t i;

	for (i = 0; i < LEPRECHN_NUM_PENS; i++)
	{
		UINT8 bk = (i & 8) ? 0x40 : 0x00;
		UINT8 r = (i & 1) ? 0xff : bk;
		UINT8 g = (i & 2) ? 0xff : bk;
		UINT8 b = (i & 4) ? 0xff : bk;

		pens[i] = MAKE_RGB(r, g, b);
	}
}



/*************************************
 *
 *  Update
 *
 *************************************/

static VIDEO_UPDATE( gameplan )
{
	gameplan_state *state = (gameplan_state *)screen->machine->driver_data;
	pen_t pens[GAMEPLAN_NUM_PENS];
	offs_t offs;

	gameplan_get_pens(pens);

	for (offs = 0; offs < state->videoram_size; offs++)
	{
		UINT8 y = offs >> 8;
		UINT8 x = offs & 0xff;

		*BITMAP_ADDR32(bitmap, y, x) = pens[state->videoram[offs] & 0x07];
	}

	return 0;
}


static VIDEO_UPDATE( leprechn )
{
	gameplan_state *state = (gameplan_state *)screen->machine->driver_data;
	pen_t pens[LEPRECHN_NUM_PENS];
	offs_t offs;

	leprechn_get_pens(pens);

	for (offs = 0; offs < state->videoram_size; offs++)
	{
		UINT8 y = offs >> 8;
		UINT8 x = offs & 0xff;

		*BITMAP_ADDR32(bitmap, y, x) = pens[state->videoram[offs]];
	}

	return 0;
}



/*************************************
 *
 *  VIA
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( video_data_w )
{
	gameplan_state *state = (gameplan_state *)device->machine->driver_data;

	state->video_data = data;
}


static WRITE8_DEVICE_HANDLER( gameplan_video_command_w )
{
	gameplan_state *state = (gameplan_state *)device->machine->driver_data;

	state->video_command = data & 0x07;
}


static WRITE8_DEVICE_HANDLER( leprechn_video_command_w )
{
	gameplan_state *state = (gameplan_state *)device->machine->driver_data;

	state->video_command = (data >> 3) & 0x07;
}


static TIMER_CALLBACK( clear_screen_done_callback )
{
	gameplan_state *state = (gameplan_state *)machine->driver_data;

	/* indicate that the we are done clearing the screen */
	via_ca1_w(state->via_0, 0);
}


static WRITE_LINE_DEVICE_HANDLER( video_command_trigger_w )
{
	gameplan_state *driver_state = (gameplan_state *)device->machine->driver_data;

	if (state == 0)
	{
		switch (driver_state->video_command)
		{
		/* draw pixel */
		case 0:
			/* auto-adjust X? */
			if (driver_state->video_data & 0x10)
			{
				if (driver_state->video_data & 0x40)
					driver_state->video_x = driver_state->video_x - 1;
				else
					driver_state->video_x = driver_state->video_x + 1;
			}

			/* auto-adjust Y? */
			if (driver_state->video_data & 0x20)
			{
				if (driver_state->video_data & 0x80)
					driver_state->video_y = driver_state->video_y - 1;
				else
					driver_state->video_y = driver_state->video_y + 1;
			}

			driver_state->videoram[driver_state->video_y * (HBSTART - HBEND) + driver_state->video_x] = driver_state->video_data & 0x0f;

			break;

		/* load X register */
		case 1:
			driver_state->video_x = driver_state->video_data;
			break;

		/* load Y register */
		case 2:
			driver_state->video_y = driver_state->video_data;
			break;

		/* clear screen */
		case 3:
			/* indicate that the we are busy */
			{
				via_ca1_w(driver_state->via_0, 1);
			}

			memset(driver_state->videoram, driver_state->video_data & 0x0f, driver_state->videoram_size);

			/* set a timer for an arbitrarily short period.
               The real time it takes to clear to screen is not
               important to the software */
			timer_call_after_resynch(device->machine, NULL, 0, clear_screen_done_callback);

			break;
		}
	}
}


static TIMER_CALLBACK( via_irq_delayed )
{
	gameplan_state *state = (gameplan_state *)machine->driver_data;
	cpu_set_input_line(state->maincpu, 0, param);
}


static void via_irq(running_device *device, int state)
{
	/* Kaos sits in a tight loop polling the VIA irq flags register, but that register is
       cleared by the irq handler. Therefore, I wait a bit before triggering the irq to
       leave time for the program to see the flag change. */
	timer_set(device->machine, ATTOTIME_IN_USEC(50), NULL, state, via_irq_delayed);
}


static READ8_DEVICE_HANDLER( vblank_r )
{
	/* this is needed for trivia quest */
	return 0x20;
}


const via6522_interface gameplan_via_0_interface =
{
	DEVCB_NULL, DEVCB_HANDLER(vblank_r),										/*inputs : A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,								/*inputs : CA/B1,CA/B2 */
	DEVCB_HANDLER(video_data_w), DEVCB_HANDLER(gameplan_video_command_w),		/*outputs: A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_LINE(video_command_trigger_w), DEVCB_NULL,	/*outputs: CA/B1,CA/B2 */
	DEVCB_LINE(via_irq)															/*irq                  */
};


const via6522_interface leprechn_via_0_interface =
{
	DEVCB_NULL, DEVCB_HANDLER(vblank_r),										/*inputs : A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,								/*inputs : CA/B1,CA/B2 */
	DEVCB_HANDLER(video_data_w), DEVCB_HANDLER(leprechn_video_command_w),		/*outputs: A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_LINE(video_command_trigger_w), DEVCB_NULL,	/*outputs: CA/B1,CA/B2 */
	DEVCB_LINE(via_irq)															/*irq                  */
};


const via6522_interface trvquest_via_0_interface =
{
	DEVCB_NULL, DEVCB_HANDLER(vblank_r),										/*inputs : A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,								/*inputs : CA/B1,CA/B2 */
	DEVCB_HANDLER(video_data_w), DEVCB_HANDLER(gameplan_video_command_w),		/*outputs: A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_LINE(video_command_trigger_w), DEVCB_NULL,	/*outputs: CA/B1,CA/B2 */
	DEVCB_NULL																	/*irq                  */
};


static TIMER_CALLBACK( via_0_ca1_timer_callback )
{
	gameplan_state *state = (gameplan_state *)machine->driver_data;

	/* !VBLANK is connected to CA1 */
	via_ca1_w(state->via_0, param);

	if (param)
		timer_adjust_oneshot(state->via_0_ca1_timer, machine->primary_screen->time_until_pos(VBSTART), 0);
	else
		timer_adjust_oneshot(state->via_0_ca1_timer, machine->primary_screen->time_until_pos(VBEND), 1);
}


/*************************************
 *
 *  Start
 *
 *************************************/

static VIDEO_START( common )
{
	gameplan_state *state = (gameplan_state *)machine->driver_data;

	state->videoram_size = (HBSTART - HBEND) * (VBSTART - VBEND);
	state->videoram = auto_alloc_array(machine, UINT8, state->videoram_size);

	state->via_0_ca1_timer = timer_alloc(machine, via_0_ca1_timer_callback, NULL);

	/* register for save states */
	state_save_register_global_pointer(machine, state->videoram, state->videoram_size);
}


static VIDEO_START( gameplan )
{
	VIDEO_START_CALL(common);
}


static VIDEO_START( leprechn )
{
	VIDEO_START_CALL(common);
}


static VIDEO_START( trvquest )
{
	VIDEO_START_CALL(common);
}



/*************************************
 *
 *  Reset
 *
 *************************************/

static VIDEO_RESET( gameplan )
{
	gameplan_state *state = (gameplan_state *)machine->driver_data;
	timer_adjust_oneshot(state->via_0_ca1_timer, machine->primary_screen->time_until_pos(VBSTART), 0);
}



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_DRIVER_START( gameplan_video )
	MDRV_VIDEO_START(gameplan)
	MDRV_VIDEO_RESET(gameplan)

	MDRV_VIDEO_START(gameplan)
	MDRV_VIDEO_UPDATE(gameplan)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(GAMEPLAN_PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( leprechn_video )
	MDRV_VIDEO_START(leprechn)
	MDRV_VIDEO_UPDATE(leprechn)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( trvquest_video )
	MDRV_IMPORT_FROM(gameplan_video)
	MDRV_VIDEO_START(trvquest)
	MDRV_VIDEO_UPDATE(gameplan)
MACHINE_DRIVER_END
