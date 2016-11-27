/*****************************************************************************

  74148 8-line-to-3-line priority encoder


  Pin layout and functions to access pins:

  input_line_w(4)  [1] /IN4   VCC [16]
  input_line_w(5)  [2] /IN5   /EO [15]  enable_output_r
  input_line_w(6)  [3] /IN6   /GS [14]  output_valid_r
  input_line_w(7)  [4] /IN7  /IN3 [13]  input_line_w(3)
  enable_input_w   [5] /EI   /IN2 [12]  input_line_w(2)
  output_r         [6] /A2   /IN1 [11]  input_line_w(1)
  output_r         [7] /A1   /IN0 [10]  input_line_w(0)
                   [8] GND   /A0  [9]   output_r


  Truth table (all logic levels indicate the actual voltage on the line):

              INPUTS            |     OUTPUTS
                                |
    EI  I0 I1 I2 I3 I4 I5 I6 I7 | A2 A1 A0 | GS EO
    ----------------------------+----------+------
 1  H   X  X  X  X  X  X  X  X  | H  H  H  | H  H
 2  L   H  H  H  H  H  H  H  H  | H  H  H  | H  L
 3  L   X  X  X  X  X  X  X  L  | L  L  L  | L  H
 4  L   X  X  X  X  X  X  L  H  | L  L  H  | L  H
 5  L   X  X  X  X  X  L  H  H  | L  H  L  | L  H
 6  L   X  X  X  X  L  H  H  H  | L  H  H  | L  H
 7  L   X  X  X  L  H  H  H  H  | H  L  L  | L  H
 8  L   X  X  L  H  H  H  H  H  | H  L  H  | L  H
 9  L   X  L  H  H  H  H  H  H  | H  H  L  | L  H
 10 L   L  H  H  H  H  H  H  H  | H  H  H  | L  H
    ----------------------------+----------+------
    L   = lo (0)
    H   = hi (1)
    X   = any state

*****************************************************************************/

#include "emu.h"
#include "machine/74148.h"


typedef struct _ttl74148_state ttl74148_state;
struct _ttl74148_state
{
	/* callback */
	void (*output_cb)(running_device *device);

	/* inputs */
	int input_lines[8];	/* pins 1-4,10-13 */
	int enable_input;	/* pin 5 */

	/* outputs */
	int output;			/* pins 6,7,9 */
	int output_valid;	/* pin 14 */
	int enable_output;	/* pin 15 */

	/* internals */
	int last_output;
	int last_output_valid;
	int last_enable_output;
};


INLINE ttl74148_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == TTL74148);

	return (ttl74148_state *)downcast<legacy_device_base *>(device)->token();
}

void ttl74148_update(running_device *device)
{
	ttl74148_state *state = get_safe_token(device);

	if (state->enable_input)
	{
		// row 1 in truth table
		state->output = 0x07;
		state->output_valid = 1;
		state->enable_output = 1;
	}
	else
	{
		int bit0, bit1, bit2;

		/* this comes straight off the data sheet schematics */
		bit0 = !(((!state->input_lines[1]) &
		           state->input_lines[2] &
		           state->input_lines[4] &
		           state->input_lines[6])  |
		         ((!state->input_lines[3]) &
		           state->input_lines[4] &
		           state->input_lines[6])  |
		         ((!state->input_lines[5]) &
		           state->input_lines[6])  |
		         (!state->input_lines[7]));

		bit1 = !(((!state->input_lines[2]) &
		           state->input_lines[4] &
		           state->input_lines[5])  |
		         ((!state->input_lines[3]) &
		           state->input_lines[4] &
		           state->input_lines[5])  |
		         (!state->input_lines[6])  |
		         (!state->input_lines[7]));

		bit2 = !((!state->input_lines[4])  |
		         (!state->input_lines[5])  |
		         (!state->input_lines[6])  |
		         (!state->input_lines[7]));

		state->output = (bit2 << 2) | (bit1 << 1) | bit0;

		state->output_valid = (state->input_lines[0] &
									 state->input_lines[1] &
									 state->input_lines[2] &
									 state->input_lines[3] &
									 state->input_lines[4] &
									 state->input_lines[5] &
									 state->input_lines[6] &
									 state->input_lines[7]);

		state->enable_output = !state->output_valid;
	}


	/* call callback if any of the outputs changed */
	if (  state->output_cb &&
		((state->output        != state->last_output) ||
	     (state->output_valid  != state->last_output_valid) ||
	     (state->enable_output != state->last_enable_output)))
	{
		state->last_output = state->output;
		state->last_output_valid = state->output_valid;
		state->last_enable_output = state->enable_output;

		state->output_cb(device);
	}
}


void ttl74148_input_line_w(running_device *device, int input_line, int data)
{
	ttl74148_state *state = get_safe_token(device);
	state->input_lines[input_line] = data ? 1 : 0;
}


void ttl74148_enable_input_w(running_device *device, int data)
{
	ttl74148_state *state = get_safe_token(device);
	state->enable_input = data ? 1 : 0;
}


int ttl74148_output_r(running_device *device)
{
	ttl74148_state *state = get_safe_token(device);
	return state->output;
}


int ttl74148_output_valid_r(running_device *device)
{
	ttl74148_state *state = get_safe_token(device);
	return state->output_valid;
}


int ttl74148_enable_output_r(running_device *device)
{
	ttl74148_state *state = get_safe_token(device);
	return state->enable_output;
}


static DEVICE_START( ttl74148 )
{
	ttl74148_config *config = (ttl74148_config *)downcast<const legacy_device_config_base &>(device->baseconfig()).inline_config();
	ttl74148_state *state = get_safe_token(device);
    state->output_cb = config->output_cb;

    state_save_register_device_item_array(device, 0, state->input_lines);
    state_save_register_device_item(device, 0, state->enable_input);
    state_save_register_device_item(device, 0, state->output);
    state_save_register_device_item(device, 0, state->output_valid);
    state_save_register_device_item(device, 0, state->enable_output);
    state_save_register_device_item(device, 0, state->last_output);
    state_save_register_device_item(device, 0, state->last_output_valid);
    state_save_register_device_item(device, 0, state->last_enable_output);
}


static DEVICE_RESET( ttl74148 )
{
	ttl74148_state *state = get_safe_token(device);

    state->enable_input = 1;
    state->input_lines[0] = 1;
    state->input_lines[1] = 1;
    state->input_lines[2] = 1;
    state->input_lines[3] = 1;
    state->input_lines[4] = 1;
    state->input_lines[5] = 1;
    state->input_lines[6] = 1;
    state->input_lines[7] = 1;

    state->last_output = -1;
    state->last_output_valid = -1;
    state->last_enable_output = -1;
}


static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##ttl74148##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET | DT_HAS_INLINE_CONFIG
#define DEVTEMPLATE_NAME		"74148"
#define DEVTEMPLATE_FAMILY		"TTL"
#include "devtempl.h"

DEFINE_LEGACY_DEVICE(TTL74148, ttl74148);
