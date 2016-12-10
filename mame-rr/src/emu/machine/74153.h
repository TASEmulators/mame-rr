/*****************************************************************************

  74153 Dual 4-line to 1-line data selectors/multiplexers


  Pin layout and functions to access pins:

  enable_w(0)        [1] /1G   VCC [16]
  b_w                [2] B     /2G [15]  enable_w(1)
  input_line_w(0,3)  [3] 1C3     A [14]  a_w
  input_line_w(0,2)  [4] 1C2   2C3 [13]  input_line_w(1,3)
  input_line_w(0,1)  [5] 1C1   2C2 [12]  input_line_w(1,2)
  input_line_w(0,0)  [6] 1C0   2C1 [11]  input_line_w(1,1)
  output_r(0)        [7] 1Y    2C0 [10]  input_line_w(1,0)
                     [8] GND    2Y [9]   output_r(1)


  Truth table (all logic levels indicate the actual voltage on the line):

            INPUTS         | OUTPUT
                           |
    G | B  A | C0 C1 C2 C3 | Y
    --+------+-------------+---
    H | X  X | X  X  X  X  | L
    L | L  L | X  X  X  X  | C0
    L | L  H | X  X  X  X  | C1
    L | H  L | X  X  X  X  | C2
    L | H  H | X  X  X  X  | C3
    --+------+-------------+---
    L   = lo (0)
    H   = hi (1)
    X   = any state

*****************************************************************************/

#ifndef TTL74153_H
#define TTL74153_H

#include "devlegcy.h"


typedef struct _ttl74153_config ttl74153_config;
struct _ttl74153_config
{
	void (*output_cb)(running_device *device);
};


#define MDRV_74153_ADD(_tag, _output_cb) \
	MDRV_DEVICE_ADD(_tag, TTL74153, 0) \
	MDRV_DEVICE_CONFIG_DATAPTR(ttl74153_config, output_cb, _output_cb)



/* must call TTL74153_update() after setting the inputs */
void ttl74153_update(running_device *device);

void ttl74153_a_w(running_device *device, int data);
void ttl74153_b_w(running_device *device, int data);
void ttl74153_input_line_w(running_device *device, int section, int input_line, int data);
void ttl74153_enable_w(running_device *device, int section, int data);
int ttl74153_output_r(running_device *device, int section);

DECLARE_LEGACY_DEVICE(TTL74153, ttl74153);

#endif
