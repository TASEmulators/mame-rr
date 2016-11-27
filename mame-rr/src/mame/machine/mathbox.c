/*
 * mathbox.c: math box simulation (Battlezone/Red Baron/Tempest)
 *
 * Copyright Eric Smith
 *
 */

#include "emu.h"
#include "mathbox.h"

#define REG0 mb->reg [0x00]
#define REG1 mb->reg [0x01]
#define REG2 mb->reg [0x02]
#define REG3 mb->reg [0x03]
#define REG4 mb->reg [0x04]
#define REG5 mb->reg [0x05]
#define REG6 mb->reg [0x06]
#define REG7 mb->reg [0x07]
#define REG8 mb->reg [0x08]
#define REG9 mb->reg [0x09]
#define REGa mb->reg [0x0a]
#define REGb mb->reg [0x0b]
#define REGc mb->reg [0x0c]
#define REGd mb->reg [0x0d]
#define REGe mb->reg [0x0e]
#define REGf mb->reg [0x0f]


#define MB_TEST 0
#define LOG(x) do { if (MB_TEST) logerror x; } while (0)

typedef struct _mathbox_state mathbox_state;
struct _mathbox_state
{
	running_device *device;
	/* math box scratch registers */
	INT16 reg[16];

	/* math box result */
	INT16 result;
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - convert a device's token
    into a mathbox_state
-------------------------------------------------*/

INLINE mathbox_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == MATHBOX);
	return (mathbox_state *)downcast<legacy_device_base *>(device)->token();
}




WRITE8_DEVICE_HANDLER( mathbox_go_w )
{
  mathbox_state *mb = get_safe_token(device);

  INT32 mb_temp;  /* temp 32-bit multiply results */
  INT16 mb_q;     /* temp used in division */
  int msb;

  LOG(("math box command %02x data %02x  ", offset, data));

  switch (offset)
    {
    case 0x00: mb->result = REG0 = (REG0 & 0xff00) | data;        break;
    case 0x01: mb->result = REG0 = (REG0 & 0x00ff) | (data << 8); break;
    case 0x02: mb->result = REG1 = (REG1 & 0xff00) | data;        break;
    case 0x03: mb->result = REG1 = (REG1 & 0x00ff) | (data << 8); break;
    case 0x04: mb->result = REG2 = (REG2 & 0xff00) | data;        break;
    case 0x05: mb->result = REG2 = (REG2 & 0x00ff) | (data << 8); break;
    case 0x06: mb->result = REG3 = (REG3 & 0xff00) | data;        break;
    case 0x07: mb->result = REG3 = (REG3 & 0x00ff) | (data << 8); break;
    case 0x08: mb->result = REG4 = (REG4 & 0xff00) | data;        break;
    case 0x09: mb->result = REG4 = (REG4 & 0x00ff) | (data << 8); break;

    case 0x0a: mb->result = REG5 = (REG5 & 0xff00) | data;        break;
      /* note: no function loads low part of REG5 without performing a computation */

    case 0x0c: mb->result = REG6 = data; break;
      /* note: no function loads high part of REG6 */

    case 0x15: mb->result = REG7 = (REG7 & 0xff00) | data;        break;
    case 0x16: mb->result = REG7 = (REG7 & 0x00ff) | (data << 8); break;

    case 0x1a: mb->result = REG8 = (REG8 & 0xff00) | data;        break;
    case 0x1b: mb->result = REG8 = (REG8 & 0x00ff) | (data << 8); break;

    case 0x0d: mb->result = REGa = (REGa & 0xff00) | data;        break;
    case 0x0e: mb->result = REGa = (REGa & 0x00ff) | (data << 8); break;
    case 0x0f: mb->result = REGb = (REGb & 0xff00) | data;        break;
    case 0x10: mb->result = REGb = (REGb & 0x00ff) | (data << 8); break;

    case 0x17: mb->result = REG7; break;
    case 0x19: mb->result = REG8; break;
    case 0x18: mb->result = REG9; break;

    case 0x0b:

      REG5 = (REG5 & 0x00ff) | (data << 8);

      REGf = (INT16)0xffff;
      REG4 -= REG2;
      REG5 -= REG3;

    step_048:

      mb_temp = ((INT32) REG0) * ((INT32) REG4);
      REGc = mb_temp >> 16;
      REGe = mb_temp & 0xffff;

      mb_temp = ((INT32) -REG1) * ((INT32) REG5);
      REG7 = mb_temp >> 16;
      mb_q = mb_temp & 0xffff;

      REG7 += REGc;

      /* rounding */
      REGe = (REGe >> 1) & 0x7fff;
      REGc = (mb_q >> 1) & 0x7fff;
      mb_q = REGc + REGe;
      if (mb_q < 0)
	REG7++;

      mb->result = REG7;

      if (REGf < 0)
	break;

      REG7 += REG2;

      /* fall into command 12 */

    case 0x12:

      mb_temp = ((INT32) REG1) * ((INT32) REG4);
      REGc = mb_temp >> 16;
      REG9 = mb_temp & 0xffff;

      mb_temp = ((INT32) REG0) * ((INT32) REG5);
      REG8 = mb_temp >> 16;
      mb_q = mb_temp & 0xffff;

      REG8 += REGc;

      /* rounding */
      REG9 = (REG9 >> 1) & 0x7fff;
      REGc = (mb_q >> 1) & 0x7fff;
      REG9 += REGc;
      if (REG9 < 0)
	REG8++;
      REG9 <<= 1;  /* why? only to get the desired load address? */

      mb->result = REG8;

      if (REGf < 0)
	break;

      REG8 += REG3;

      REG9 &= 0xff00;

      /* fall into command 13 */

    case 0x13:
      LOG(("\nR7: %04x  R8: %04x  R9: %04x\n", REG7, REG8, REG9));

      REGc = REG9;
      mb_q = REG8;
      goto step_0bf;

    case 0x14:
      REGc = REGa;
      mb_q = REGb;

    step_0bf:
      REGe = REG7 ^ mb_q;  /* save sign of result */
      REGd = mb_q;
      if (mb_q >= 0)
	mb_q = REGc;
      else
	{
	  REGd = - mb_q - 1;
	  mb_q = - REGc - 1;
	  if ((mb_q < 0) && ((mb_q + 1) < 0))
	    REGd++;
	  mb_q++;
	}

    /* step 0c9: */
      /* REGc = abs (REG7) */
      if (REG7 >= 0)
	REGc = REG7;
      else
        REGc = -REG7;

      REGf = REG6;  /* step counter */

      do
	{
	  REGd -= REGc;
	  msb = ((mb_q & 0x8000) != 0);
	  mb_q <<= 1;
	  if (REGd >= 0)
	    mb_q++;
	  else
	    REGd += REGc;
	  REGd <<= 1;
	  REGd += msb;
	}
      while (--REGf >= 0);

      if (REGe >= 0)
	mb->result = mb_q;
      else
	mb->result = - mb_q;
      break;

    case 0x11:
      REG5 = (REG5 & 0x00ff) | (data << 8);
      REGf = 0x0000;  /* do everything in one step */
      goto step_048;
      break;

    case 0x1c:
      /* window test? */
      REG5 = (REG5 & 0x00ff) | (data << 8);
      do
	{
	  REGe = (REG4 + REG7) >> 1;
	  REGf = (REG5 + REG8) >> 1;
	  if ((REGb < REGe) && (REGf < REGe) && ((REGe + REGf) >= 0))
	    { REG7 = REGe; REG8 = REGf; }
	  else
	    { REG4 = REGe; REG5 = REGf; }
	}
      while (--REG6 >= 0);

      mb->result = REG8;
      break;

    case 0x1d:
      REG3 = (REG3 & 0x00ff) | (data << 8);

      REG2 -= REG0;
      if (REG2 < 0)
	REG2 = -REG2;

      REG3 -= REG1;
      if (REG3 < 0)
	REG3 = -REG3;

      /* fall into command 1e */

    case 0x1e:
      /* result = max (REG2, REG3) + 3/8 * min (REG2, REG3) */
      if (REG3 >= REG2)
        { REGc = REG2; REGd = REG3; }
      else
	{ REGd = REG2; REGc = REG3; }
      REGc >>= 2;
      REGd += REGc;
      REGc >>= 1;
      mb->result = REGd = (REGc + REGd);
      break;

    case 0x1f:
      logerror("math box function 0x1f\n");
      /* $$$ do some computation here (selftest? signature analysis? */
      break;
    }

  LOG(("  result %04x\n", mb->result & 0xffff));
}

READ8_DEVICE_HANDLER( mathbox_status_r )
{
	return 0x00; /* always done! */
}

READ8_DEVICE_HANDLER( mathbox_lo_r )
{
	mathbox_state *mb = get_safe_token(device);

	return mb->result & 0xff;
}

READ8_DEVICE_HANDLER( mathbox_hi_r )
{
	mathbox_state *mb = get_safe_token(device);

	return (mb->result >> 8) & 0xff;
}



/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    mathbox_portb_r - return port B output
    value
-------------------------------------------------*/

static DEVICE_START( mathbox )
{
	mathbox_state *mb = get_safe_token(device);

	/* validate arguments */
	assert(device != NULL);

	/* set static values */
	mb->device = device;

	/* register for save states */
	state_save_register_device_item(device, 0, mb->result);
	state_save_register_device_item_array(device, 0, mb->reg);
}


static DEVICE_RESET( mathbox )
{
	mathbox_state *mb = get_safe_token(device);

	mb->result = 0;
	memset(mb->reg, 0, sizeof(INT16)*16);
}


DEVICE_GET_INFO( mathbox )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(mathbox_state);		break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(mathbox);break;
		case DEVINFO_FCT_STOP:							/* Nothing */							break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(mathbox);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "MATHBOX");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "I/O devices");			break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_DEVICE(MATHBOX, mathbox);
