/***************************************************************************

    Namco 51XX

    This custom chip is a Fujitsu MB8843 MCU programmed to act as an I/O
    device with built-in coin management. It is also apparently used as a
    protection device. It keeps track of the players scores, and checks
    if a high score has been obtained or bonus lives should be awarded.
    The main CPU has a range of commands to increment/decrement the score
    by various fixed amounts.

    The device is used to its full potential only by Bosconian; Xevious
    uses it too, but only to do a protection check on startup.

    CMD = command from main CPU
    ANS = answer to main CPU

    The chip reads/writes the I/O ports when the /IRQ is pulled down.
    Pin 21 determines whether a read or write should happen (1=R, 0=W).

               +------+
             EX|1   42|Vcc
              X|2   41|K3
         /RESET|3   40|K2
           /IRQ|4   39|K1
             SO|5   38|K0
             SI|6   37|R15
        /SC /TO|7   36|R14
            /TC|8   35|R13
             P0|9   34|R12
             P1|10  33|R11
             P2|11  32|R10
             P3|12  31|R9
             O0|13  30|R8
             O1|14  29|R7
             O2|15  28|R6
             O3|16  27|R5
             O4|17  26|R4
             O5|18  25|R3
             O6|19  24|R2
             O7|20  23|R1
            GND|21  22|R0
               +------+

    commands:
    00: nop
    01 + 4 arguments: set coinage (xevious, possibly because of a bug, is different)
    02: go in "credit" mode and enable start buttons
    03: disable joystick remapping
    04: enable joystick remapping
    05: go in "switch" mode
    06: nop
    07: nop

***************************************************************************/

#include "emu.h"
#include "namco51.h"
#include "cpu/mb88xx/mb88xx.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


#define READ_PORT(st,num)			devcb_call_read8(&(st)->in[num], 0)
#define WRITE_PORT(st,num,data) 	devcb_call_write8(&(st)->out[num], 0, data)


typedef struct _namco_51xx_state namco_51xx_state;
struct _namco_51xx_state
{
	running_device *	cpu;
	devcb_resolved_read8 in[4];
	devcb_resolved_write8 out[2];
	INT32 lastcoins,lastbuttons;
	INT32 credits;
	INT32 coins[2];
	INT32 coins_per_cred[2];
	INT32 creds_per_coin[2];
	INT32 in_count;
	INT32 mode,coincred_mode,remap_joy;
};

INLINE namco_51xx_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == NAMCO_51XX);

	return (namco_51xx_state *)downcast<legacy_device_base *>(device)->token();
}



WRITE8_DEVICE_HANDLER( namco_51xx_write )
{
	namco_51xx_state *state = get_safe_token(device);

	data &= 0x07;

	LOG(("%s: custom 51XX write %02x\n",cpuexec_describe_context(device->machine),data));

	if (state->coincred_mode)
	{
		switch (state->coincred_mode--)
		{
			case 4: state->coins_per_cred[0] = data; break;
			case 3: state->creds_per_coin[0] = data; break;
			case 2: state->coins_per_cred[1] = data; break;
			case 1: state->creds_per_coin[1] = data; break;
		}
	}
	else
	{
		switch (data)
		{
			case 0:	// nop
				break;

			case 1:	// set coinage
				state->coincred_mode = 4;
				/* this is a good time to reset the credits counter */
				state->credits = 0;

				{
					/* kludge for a possible bug in Xevious */
					static const game_driver *namcoio_51XX_driver = NULL;
					static int namcoio_51XX_kludge = 0;

					/* Only compute namcoio_51XX_kludge when gamedrv changes */
					if (namcoio_51XX_driver != device->machine->gamedrv)
					{
						namcoio_51XX_driver = device->machine->gamedrv;
						if (strcmp(namcoio_51XX_driver->name, "xevious") == 0 ||
							strcmp(namcoio_51XX_driver->parent, "xevious") == 0)
							namcoio_51XX_kludge = 1;
						else
							namcoio_51XX_kludge = 0;
					}

					if (namcoio_51XX_kludge)
					{
						state->coincred_mode = 6;
						state->remap_joy = 1;
					}
				}
				break;

			case 2:	// go in "credits" mode and enable start buttons
				state->mode = 1;
				state->in_count = 0;
				break;

			case 3:	// disable joystick remapping
				state->remap_joy = 0;
				break;

			case 4:	// enable joystick remapping
				state->remap_joy = 1;
				break;

			case 5:	// go in "switch" mode
				state->mode = 0;
				state->in_count = 0;
				break;

			default:
				logerror("unknown 51XX command %02x\n",data);
				break;
		}
	}
}


/* joystick input mapping

  The joystick is parsed and a number corresponding to the direction is returned,
  according to the following table:

          0
        7   1
      6   8   2
        5   3
          4

  The values for directions impossible to obtain on a joystick have not been
  verified on Namco original hardware, but they are the same in all the bootlegs,
  so we can assume they are right.
*/
static const int joy_map[16] =
/*  LDRU, LDR, LDU,  LD, LRU, LR,  LU,    L, DRU,  DR,  DU,   D,  RU,   R,   U, center */
{	 0xf, 0xe, 0xd, 0x5, 0xc, 0x9, 0x7, 0x6, 0xb, 0x3, 0xa, 0x4, 0x1, 0x2, 0x0, 0x8 };


READ8_DEVICE_HANDLER( namco_51xx_read )
{
	namco_51xx_state *state = get_safe_token(device);

	LOG(("%s: custom 51XX read\n",cpuexec_describe_context(device->machine)));

	if (state->mode == 0)	/* switch mode */
	{
		switch ((state->in_count++) % 3)
		{
			default:
			case 0: return READ_PORT(state,0) | (READ_PORT(state,1) << 4);
			case 1: return READ_PORT(state,2) | (READ_PORT(state,3) << 4);
			case 2: return 0;	// nothing?
		}
	}
	else	/* credits mode */
	{
		switch ((state->in_count++) % 3)
		{
			default:
			case 0:	// number of credits in BCD format
				{
					int in,toggle;

					in = ~(READ_PORT(state,0) | (READ_PORT(state,1) << 4));
					toggle = in ^ state->lastcoins;
					state->lastcoins = in;

					if (state->coins_per_cred[0] > 0)
					{
						if (state->credits >= 99)
						{
							WRITE_PORT(state,1,1);	// coin lockout
						}
						else
						{
							WRITE_PORT(state,1,0);	// coin lockout
							/* check if the user inserted a coin */
							if (toggle & in & 0x10)
							{
								state->coins[0]++;
								WRITE_PORT(state,0,0x04);	// coin counter
								WRITE_PORT(state,0,0x0c);
								if (state->coins[0] >= state->coins_per_cred[0])
								{
									state->credits += state->creds_per_coin[0];
									state->coins[0] -= state->coins_per_cred[0];
								}
							}
							if (toggle & in & 0x20)
							{
								state->coins[1]++;
								WRITE_PORT(state,0,0x08);	// coin counter
								WRITE_PORT(state,0,0x0c);
								if (state->coins[1] >= state->coins_per_cred[1])
								{
									state->credits += state->creds_per_coin[1];
									state->coins[1] -= state->coins_per_cred[1];
								}
							}
							if (toggle & in & 0x40)
							{
								state->credits++;
							}
						}
					}
					else state->credits = 100;	// free play

					if (state->mode == 1)
					{
						int on = (device->machine->primary_screen->frame_number() & 0x10) >> 4;

						if (state->credits >= 2)
							WRITE_PORT(state,0,0x0c | 3*on);	// lamps
						else if (state->credits >= 1)
							WRITE_PORT(state,0,0x0c | 2*on);	// lamps
						else
							WRITE_PORT(state,0,0x0c);	// lamps off

						/* check for 1 player start button */
						if (toggle & in & 0x04)
						{
							if (state->credits >= 1)
							{
								state->credits--;
								state->mode = 2;
								WRITE_PORT(state,0,0x0c);	// lamps off
							}
						}
						/* check for 2 players start button */
						else if (toggle & in & 0x08)
						{
							if (state->credits >= 2)
							{
								state->credits -= 2;
								state->mode = 2;
								WRITE_PORT(state, 0,0x0c);	// lamps off
							}
						}
					}
				}

				if (~READ_PORT(state, 1) & 0x08)	/* check test mode switch */
					return 0xbb;

				return (state->credits / 10) * 16 + state->credits % 10;

			case 1:
				{
					int joy = READ_PORT(state,2) & 0x0f;
					int in,toggle;

					in = ~READ_PORT(state,0);
					toggle = in ^ state->lastbuttons;
					state->lastbuttons = (state->lastbuttons & 2) | (in & 1);

					/* remap joystick */
					if (state->remap_joy) joy = joy_map[joy];

					/* fire */
					joy |= ((toggle & in & 0x01)^1) << 4;
					joy |= ((in & 0x01)^1) << 5;

					return joy;
				}

			case 2:
				{
					int joy = READ_PORT(state,3) & 0x0f;
					int in,toggle;

					in = ~READ_PORT(state,0);
					toggle = in ^ state->lastbuttons;
					state->lastbuttons = (state->lastbuttons & 1) | (in & 2);

					/* remap joystick */
					if (state->remap_joy) joy = joy_map[joy];

					/* fire */
					joy |= ((toggle & in & 0x02)^2) << 3;
					joy |= ((in & 0x02)^2) << 4;

					return joy;
				}
		}
	}
}





/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

static ADDRESS_MAP_START( namco_51xx_map_io, ADDRESS_SPACE_IO, 8 )
//  AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(namco_51xx_K_r)
//  AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(namco_51xx_O_w)
//  AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READ(namco_51xx_R0_r)
//  AM_RANGE(MB88_PORTR2, MB88_PORTR2) AM_READ(namco_51xx_R2_r)
ADDRESS_MAP_END


static MACHINE_DRIVER_START( namco_51xx )
	MDRV_CPU_ADD("mcu", MB8843, DERIVED_CLOCK(1,1))		/* parent clock, internally divided by 6 */
	MDRV_CPU_IO_MAP(namco_51xx_map_io)
	MDRV_DEVICE_DISABLE()
MACHINE_DRIVER_END


ROM_START( namco_51xx )
	ROM_REGION( 0x400, "mcu", ROMREGION_LOADBYNAME )
	ROM_LOAD( "51xx.bin",     0x0000, 0x0400, CRC(c2f57ef8) SHA1(50de79e0d6a76bda95ffb02fcce369a79e6abfec) )
ROM_END


/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( namco_51xx )
{
	const namco_51xx_interface *config = (const namco_51xx_interface *)device->baseconfig().static_config();
	namco_51xx_state *state = get_safe_token(device);
	astring tempstring;

	assert(config != NULL);

	/* find our CPU */
	state->cpu = device->subdevice("mcu");
	assert(state->cpu != NULL);

	/* resolve our read callbacks */
	devcb_resolve_read8(&state->in[0], &config->in[0], device);
	devcb_resolve_read8(&state->in[1], &config->in[1], device);
	devcb_resolve_read8(&state->in[2], &config->in[2], device);
	devcb_resolve_read8(&state->in[3], &config->in[3], device);

	/* resolve our write callbacks */
	devcb_resolve_write8(&state->out[0], &config->out[0], device);
	devcb_resolve_write8(&state->out[1], &config->out[1], device);
#if 0
	INT32 lastcoins,lastbuttons;
	INT32 credits;
	INT32 coins[2];
	INT32 coins_per_cred[2];
	INT32 creds_per_coin[2];
	INT32 in_count;
	INT32 mode,coincred_mode,remap_joy;
#endif
	state_save_register_device_item(device, 0, state->lastcoins);
	state_save_register_device_item(device, 0, state->lastbuttons);
	state_save_register_device_item(device, 0, state->credits);
	state_save_register_device_item_array(device, 0, state->coins);
	state_save_register_device_item_array(device, 0, state->coins_per_cred);
	state_save_register_device_item_array(device, 0, state->creds_per_coin);
	state_save_register_device_item(device, 0, state->in_count);
	state_save_register_device_item(device, 0, state->mode);
	state_save_register_device_item(device, 0, state->coincred_mode);
	state_save_register_device_item(device, 0, state->remap_joy);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( namco_51xx )
{
	namco_51xx_state *state = get_safe_token(device);

	/* reset internal registers */
	state->credits = 0;
	state->coins[0] = 0;
	state->coins_per_cred[0] = 1;
	state->creds_per_coin[0] = 1;
	state->coins[1] = 0;
	state->coins_per_cred[1] = 1;
	state->creds_per_coin[1] = 1;
	state->in_count = 0;
}


/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##namco_51xx##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET | DT_HAS_ROM_REGION | DT_HAS_MACHINE_CONFIG
#define DEVTEMPLATE_NAME		"Namco 51xx"
#define DEVTEMPLATE_FAMILY		"Namco I/O"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(NAMCO_51XX, namco_51xx);
