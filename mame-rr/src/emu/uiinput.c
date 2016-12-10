/***************************************************************************

    uiinput.c

    Internal MAME user interface input state.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "uiinput.h"
#include "profiler.h"
#include "render.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define EVENT_QUEUE_SIZE		128

enum
{
	SEQ_PRESSED_FALSE = 0,		/* not pressed */
	SEQ_PRESSED_TRUE,			/* pressed */
	SEQ_PRESSED_RESET			/* reset -- converted to FALSE once detected as not pressed */
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* private input port state */
struct _ui_input_private
{
	/* pressed states; retrieved with ui_input_pressed() */
	osd_ticks_t					next_repeat[__ipt_max];
	UINT8						seqpressed[__ipt_max];

	/* mouse position/info */
	render_target *				current_mouse_target;
	INT32						current_mouse_x;
	INT32						current_mouse_y;
	int							current_mouse_down;

	/* popped states; ring buffer of ui_events */
	ui_event					events[EVENT_QUEUE_SIZE];
	int							events_start;
	int							events_end;
};



/***************************************************************************
    FUNCTION PROTOYPES
***************************************************************************/

//static void ui_input_frame_update(running_machine *machine);



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    ui_input_init - initializes the UI input
    system
-------------------------------------------------*/

void ui_input_init(running_machine *machine)
{
	/* create the private data */
	machine->ui_input_data = auto_alloc_clear(machine, ui_input_private);
	machine->ui_input_data->current_mouse_x = -1;
	machine->ui_input_data->current_mouse_y = -1;

	/* add a frame callback to poll inputs */
	machine->add_notifier(MACHINE_NOTIFY_FRAME, ui_input_frame_update);
}



/***************************************************************************
    EVENT HANDLING
***************************************************************************/

/*-------------------------------------------------
    ui_input_frame_update - looks through pressed
    input as per events pushed our way and posts
    corresponding IPT_UI_* events
-------------------------------------------------*/

void ui_input_frame_update(running_machine &machine)
{
	ui_input_private *uidata = machine.ui_input_data;
	int code;

	/* update the state of all the UI keys */
	for (code = __ipt_ui_start; code <= __ipt_ui_end; code++)
	{
		int pressed = input_seq_pressed(&machine, input_type_seq(&machine, code, 0, SEQ_TYPE_STANDARD));
		if (!pressed || uidata->seqpressed[code] != SEQ_PRESSED_RESET)
			uidata->seqpressed[code] = pressed;
	}
}


/*-------------------------------------------------
    ui_input_push_event - pushes a single event
    onto the queue
-------------------------------------------------*/

int ui_input_push_event(running_machine *machine, ui_event evt)
{
	ui_input_private *uidata = machine->ui_input_data;

	/* we may be called before the UI is initialized */
	if (uidata == NULL)
		return FALSE;

	/* some pre-processing (this is an icky place to do this stuff!) */
	switch (evt.event_type)
	{
		case UI_EVENT_MOUSE_MOVE:
			uidata->current_mouse_target = evt.target;
			uidata->current_mouse_x = evt.mouse_x;
			uidata->current_mouse_y = evt.mouse_y;
			break;

		case UI_EVENT_MOUSE_LEAVE:
			if (uidata->current_mouse_target == evt.target)
			{
				uidata->current_mouse_target = NULL;
				uidata->current_mouse_x = -1;
				uidata->current_mouse_y = -1;
			}
			break;

		case UI_EVENT_MOUSE_DOWN:
			uidata->current_mouse_down = TRUE;
			break;

		case UI_EVENT_MOUSE_UP:
			uidata->current_mouse_down = FALSE;
			break;

		default:
			/* do nothing */
			break;
	}

	/* is the queue filled up? */
	if ((uidata->events_end + 1) % ARRAY_LENGTH(uidata->events) == uidata->events_start)
		return FALSE;

	uidata->events[uidata->events_end++] = evt;
	uidata->events_end %= ARRAY_LENGTH(uidata->events);
	return TRUE;
}


/*-------------------------------------------------
    ui_input_pop_event - pops an event off of the queue
-------------------------------------------------*/

int ui_input_pop_event(running_machine *machine, ui_event *evt)
{
	ui_input_private *uidata = machine->ui_input_data;
	int result;

	if (uidata->events_start != uidata->events_end)
	{
		*evt = uidata->events[uidata->events_start++];
		uidata->events_start %= ARRAY_LENGTH(uidata->events);
		result = TRUE;
	}
	else
	{
		memset(evt, 0, sizeof(*evt));
		result = FALSE;
	}
	return result;
}


/*-------------------------------------------------
    ui_input_reset - clears all outstanding events
    and resets the sequence states
-------------------------------------------------*/

void ui_input_reset(running_machine *machine)
{
	ui_input_private *uidata = machine->ui_input_data;
	int code;

	uidata->events_start = 0;
	uidata->events_end = 0;
	for (code = __ipt_ui_start; code <= __ipt_ui_end; code++)
	{
		uidata->seqpressed[code] = SEQ_PRESSED_RESET;
		uidata->next_repeat[code] = 0;
	}
}


/*-------------------------------------------------
    ui_input_find_mouse - retrieves the current
    location of the mouse
-------------------------------------------------*/

render_target *ui_input_find_mouse(running_machine *machine, INT32 *x, INT32 *y, int *button)
{
	ui_input_private *uidata = machine->ui_input_data;
	if (x != NULL)
		*x = uidata->current_mouse_x;
	if (y != NULL)
		*y = uidata->current_mouse_y;
	if (button != NULL)
		*button = uidata->current_mouse_down;
	return uidata->current_mouse_target;
}



/***************************************************************************
    USER INTERFACE SEQUENCE READING
***************************************************************************/

/*-------------------------------------------------
    ui_input_pressed - return TRUE if a key down
    for the given user interface sequence is
    detected
-------------------------------------------------*/

int ui_input_pressed(running_machine *machine, int code)
{
	return ui_input_pressed_repeat(machine, code, 0);
}


/*-------------------------------------------------
    ui_input_pressed_repeat - return TRUE if a key
    down for the given user interface sequence is
    detected, or if autorepeat at the given speed
    is triggered
-------------------------------------------------*/

int ui_input_pressed_repeat(running_machine *machine, int code, int speed)
{
	ui_input_private *uidata = machine->ui_input_data;
	int pressed = FALSE;

profiler_mark_start(PROFILER_INPUT);

	/* get the status of this key (assumed to be only in the defaults) */
	assert(code >= IPT_UI_CONFIGURE && code <= IPT_OSD_16);
	pressed = (uidata->seqpressed[code] == SEQ_PRESSED_TRUE);

	/* if down, handle it specially */
	if (pressed)
	{
		osd_ticks_t tps = osd_ticks_per_second();

		/* if this is the first press, set a 3x delay and leave pressed = 1 */
		if (uidata->next_repeat[code] == 0)
			uidata->next_repeat[code] = osd_ticks() + 3 * speed * tps / 60;

		/* if this is an autorepeat case, set a 1x delay and leave pressed = 1 */
		else if (speed > 0 && (osd_ticks() + tps - uidata->next_repeat[code]) >= tps)
			uidata->next_repeat[code] += 1 * speed * tps / 60;

		/* otherwise, reset pressed = 0 */
		else
			pressed = FALSE;
	}

	/* if we're not pressed, reset the memory field */
	else
		uidata->next_repeat[code] = 0;

profiler_mark_end();

	return pressed;
}
