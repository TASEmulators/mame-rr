/***************************************************************************

TC0220IOC
---------
A simple I/O interface with integrated watchdog.
It has four address inputs, which would suggest 16 bytes of addressing space,
but only the first 8 seem to be used.

000 R  IN00-07 (DSA)
000  W watchdog reset
001 R  IN08-15 (DSB)
002 R  IN16-23 (1P)
002  W unknown. Usually written on startup: initialize?
003 R  IN24-31 (2P)
004 RW coin counters and lockout
005  W unknown
006  W unknown
007 R  INB0-7 (coin)


TC0510NIO
---------
Newer version of the I/O chip

000 R  DSWA
000  W watchdog reset
001 R  DSWB
001  W unknown (ssi)
002 R  1P
003 R  2P
003  W unknown (yuyugogo, qzquest and qzchikyu use it a lot)
004 RW coin counters and lockout
005  W unknown
006  W unknown (koshien and pulirula use it a lot)
007 R  coin


TC0640FIO
---------
Newer version of the I/O chip ?


***************************************************************************/

#include "emu.h"
#include "machine/taitoio.h"


/***************************************************************************/
/*                                                                         */
/*                              TC0220IOC                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _tc0220ioc_state tc0220ioc_state;
struct _tc0220ioc_state
{
	UINT8      regs[8];
	UINT8      port;

	devcb_resolved_read8	read_0;
	devcb_resolved_read8	read_1;
	devcb_resolved_read8	read_2;
	devcb_resolved_read8	read_3;
	devcb_resolved_read8	read_7;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0220ioc_state *tc0220ioc_get_safe_token( running_device *device )
{
	assert(device != NULL);
	assert(device->type() == TC0220IOC);

	return (tc0220ioc_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const tc0220ioc_interface *tc0220ioc_get_interface( running_device *device )
{
	assert(device != NULL);
	assert((device->type() == TC0220IOC));
	return (const tc0220ioc_interface *) device->baseconfig().static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_DEVICE_HANDLER( tc0220ioc_r )
{
	tc0220ioc_state *tc0220ioc =  tc0220ioc_get_safe_token(device);

	switch (offset)
	{
		case 0x00:
			return devcb_call_read8(&tc0220ioc->read_0, 0);

		case 0x01:
			return devcb_call_read8(&tc0220ioc->read_1, 0);

		case 0x02:
			return devcb_call_read8(&tc0220ioc->read_2, 0);

		case 0x03:
			return devcb_call_read8(&tc0220ioc->read_3, 0);

		case 0x04:	/* coin counters and lockout */
			return tc0220ioc->regs[4];

		case 0x07:
			return devcb_call_read8(&tc0220ioc->read_7, 0);

		default:
//logerror("PC %06x: warning - read TC0220IOC address %02x\n",cpu_get_pc(space->cpu),offset);
			return 0xff;
	}
}

WRITE8_DEVICE_HANDLER( tc0220ioc_w )
{
	tc0220ioc_state *tc0220ioc =  tc0220ioc_get_safe_token(device);

	tc0220ioc->regs[offset] = data;
	switch (offset)
	{

		case 0x00:
			watchdog_reset(device->machine);
			break;

		case 0x04:	/* coin counters and lockout, hi nibble irrelevant */

			coin_lockout_w(device->machine, 0, ~data & 0x01);
			coin_lockout_w(device->machine, 1, ~data & 0x02);
			coin_counter_w(device->machine, 0, data & 0x04);
			coin_counter_w(device->machine, 1, data & 0x08);

//if (data & 0xf0)
//logerror("PC %06x: warning - write %02x to TC0220IOC address %02x\n",cpu_get_pc(space->cpu),data,offset);

			break;

		default:
//logerror("PC %06x: warning - write %02x to TC0220IOC address %02x\n",cpu_get_pc(space->cpu),data,offset);
			break;
	}
}

READ8_DEVICE_HANDLER( tc0220ioc_port_r )
{
	tc0220ioc_state *tc0220ioc =  tc0220ioc_get_safe_token(device);
	return tc0220ioc->port;
}

WRITE8_DEVICE_HANDLER( tc0220ioc_port_w )
{
	tc0220ioc_state *tc0220ioc =  tc0220ioc_get_safe_token(device);
	tc0220ioc->port = data;
}

READ8_DEVICE_HANDLER( tc0220ioc_portreg_r )
{
	tc0220ioc_state *tc0220ioc =  tc0220ioc_get_safe_token(device);
	return tc0220ioc_r(device, tc0220ioc->port);
}

WRITE8_DEVICE_HANDLER( tc0220ioc_portreg_w )
{
	tc0220ioc_state *tc0220ioc =  tc0220ioc_get_safe_token(device);
	tc0220ioc_w(device, tc0220ioc->port, data);
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0220ioc )
{
	tc0220ioc_state *tc0220ioc =  tc0220ioc_get_safe_token(device);
	const tc0220ioc_interface *intf = tc0220ioc_get_interface(device);

	devcb_resolve_read8(&tc0220ioc->read_0, &intf->read_0, device);
	devcb_resolve_read8(&tc0220ioc->read_1, &intf->read_1, device);
	devcb_resolve_read8(&tc0220ioc->read_2, &intf->read_2, device);
	devcb_resolve_read8(&tc0220ioc->read_3, &intf->read_3, device);
	devcb_resolve_read8(&tc0220ioc->read_7, &intf->read_7, device);

	state_save_register_device_item_array(device, 0, tc0220ioc->regs);
	state_save_register_device_item(device, 0, tc0220ioc->port);
}

static DEVICE_RESET( tc0220ioc )
{
	tc0220ioc_state *tc0220ioc =  tc0220ioc_get_safe_token(device);
	int i;

	tc0220ioc->port = 0;

	for (i = 0; i < 8; i++)
		tc0220ioc->regs[i] = 0;
}


/***************************************************************************/
/*                                                                         */
/*                              TC0510NIO                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _tc0510nio_state tc0510nio_state;
struct _tc0510nio_state
{
	UINT8   regs[8];

	devcb_resolved_read8	read_0;
	devcb_resolved_read8	read_1;
	devcb_resolved_read8	read_2;
	devcb_resolved_read8	read_3;
	devcb_resolved_read8	read_7;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0510nio_state *tc0510nio_get_safe_token( running_device *device )
{
	assert(device != NULL);
	assert(device->type() == TC0510NIO);

	return (tc0510nio_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const tc0510nio_interface *tc0510nio_get_interface( running_device *device )
{
	assert(device != NULL);
	assert((device->type() == TC0510NIO));
	return (const tc0510nio_interface *) device->baseconfig().static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_DEVICE_HANDLER( tc0510nio_r )
{
	tc0510nio_state *tc0510nio = tc0510nio_get_safe_token(device);

	switch (offset)
	{
		case 0x00:
			return devcb_call_read8(&tc0510nio->read_0, 0);

		case 0x01:
			return devcb_call_read8(&tc0510nio->read_1, 0);

		case 0x02:
			return devcb_call_read8(&tc0510nio->read_2, 0);

		case 0x03:
			return devcb_call_read8(&tc0510nio->read_3, 0);

		case 0x04:	/* coin counters and lockout */
			return tc0510nio->regs[4];

		case 0x07:
			return devcb_call_read8(&tc0510nio->read_7, 0);

		default:
//logerror("PC %06x: warning - read TC0510NIO address %02x\n",cpu_get_pc(space->cpu),offset);
			return 0xff;
	}
}

WRITE8_DEVICE_HANDLER( tc0510nio_w )
{
	tc0510nio_state *tc0510nio = tc0510nio_get_safe_token(device);

	tc0510nio->regs[offset] = data;

	switch (offset)
	{
		case 0x00:
			watchdog_reset(device->machine);
			break;

		case 0x04:	/* coin counters and lockout */
			coin_lockout_w(device->machine, 0, ~data & 0x01);
			coin_lockout_w(device->machine, 1, ~data & 0x02);
			coin_counter_w(device->machine, 0, data & 0x04);
			coin_counter_w(device->machine, 1, data & 0x08);
			break;

		default:
//logerror("PC %06x: warning - write %02x to TC0510NIO address %02x\n",cpu_get_pc(space->cpu),data,offset);
			break;
	}
}

READ16_DEVICE_HANDLER( tc0510nio_halfword_r )
{
	return tc0510nio_r(device, offset);
}

WRITE16_DEVICE_HANDLER( tc0510nio_halfword_w )
{
	if (ACCESSING_BITS_0_7)
		tc0510nio_w(device, offset, data & 0xff);
	else
	{
		/* driftout writes the coin counters here - bug? */
//logerror("CPU #0 PC %06x: warning - write to MSB of TC0510NIO address %02x\n",cpu_get_pc(space->cpu),offset);
		tc0510nio_w(device, offset, (data >> 8) & 0xff);
	}
}

READ16_DEVICE_HANDLER( tc0510nio_halfword_wordswap_r )
{
	return tc0510nio_halfword_r(device, offset ^ 1, mem_mask);
}

WRITE16_DEVICE_HANDLER( tc0510nio_halfword_wordswap_w )
{
	tc0510nio_halfword_w(device, offset ^ 1,data, mem_mask);
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0510nio )
{
	tc0510nio_state *tc0510nio =  tc0510nio_get_safe_token(device);
	const tc0510nio_interface *intf = tc0510nio_get_interface(device);

	devcb_resolve_read8(&tc0510nio->read_0, &intf->read_0, device);
	devcb_resolve_read8(&tc0510nio->read_1, &intf->read_1, device);
	devcb_resolve_read8(&tc0510nio->read_2, &intf->read_2, device);
	devcb_resolve_read8(&tc0510nio->read_3, &intf->read_3, device);
	devcb_resolve_read8(&tc0510nio->read_7, &intf->read_7, device);

	state_save_register_device_item_array(device, 0, tc0510nio->regs);
}

static DEVICE_RESET( tc0510nio )
{
	tc0510nio_state *tc0510nio =  tc0510nio_get_safe_token(device);
	int i;

	for (i = 0; i < 8; i++)
		tc0510nio->regs[i] = 0;
}

/***************************************************************************/
/*                                                                         */
/*                              TC0640FIO                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _tc0640fio_state tc0640fio_state;
struct _tc0640fio_state
{
	UINT8   regs[8];

	devcb_resolved_read8	read_0;
	devcb_resolved_read8	read_1;
	devcb_resolved_read8	read_2;
	devcb_resolved_read8	read_3;
	devcb_resolved_read8	read_7;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0640fio_state *tc0640fio_get_safe_token( running_device *device )
{
	assert(device != NULL);
	assert(device->type() == TC0640FIO);

	return (tc0640fio_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const tc0640fio_interface *tc0640fio_get_interface( running_device *device )
{
	assert(device != NULL);
	assert((device->type() == TC0640FIO));
	return (const tc0640fio_interface *) device->baseconfig().static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_DEVICE_HANDLER( tc0640fio_r )
{
	tc0640fio_state *tc0640fio = tc0640fio_get_safe_token(device);

	switch (offset)
	{
		case 0x00:
			return devcb_call_read8(&tc0640fio->read_0, 0);

		case 0x01:
			return devcb_call_read8(&tc0640fio->read_1, 0);

		case 0x02:
			return devcb_call_read8(&tc0640fio->read_2, 0);

		case 0x03:
			return devcb_call_read8(&tc0640fio->read_3, 0);

		case 0x04:	/* coin counters and lockout */
			return tc0640fio->regs[4];

		case 0x07:
			return devcb_call_read8(&tc0640fio->read_7, 0);

		default:
//logerror("PC %06x: warning - read TC0640FIO address %02x\n",cpu_get_pc(space->cpu),offset);
			return 0xff;
	}
}

WRITE8_DEVICE_HANDLER( tc0640fio_w )
{
	tc0640fio_state *tc0640fio = tc0640fio_get_safe_token(device);

	tc0640fio->regs[offset] = data;
	switch (offset)
	{

		case 0x00:
			watchdog_reset(device->machine);
			break;

		case 0x04:	/* coin counters and lockout */
			coin_lockout_w(device->machine, 0, ~data & 0x01);
			coin_lockout_w(device->machine, 1, ~data & 0x02);
			coin_counter_w(device->machine, 0, data & 0x04);
			coin_counter_w(device->machine, 1, data & 0x08);
			break;

		default:
//logerror("PC %06x: warning - write %02x to TC0640FIO address %02x\n",cpu_get_pc(space->cpu),data,offset);
			break;
	}
}

READ16_DEVICE_HANDLER( tc0640fio_halfword_r )
{
	return tc0640fio_r(device, offset);
}

WRITE16_DEVICE_HANDLER( tc0640fio_halfword_w )
{
	if (ACCESSING_BITS_0_7)
		tc0640fio_w(device, offset, data & 0xff);
	else
	{
		tc0640fio_w(device, offset, (data >> 8) & 0xff);
//logerror("CPU #0 PC %06x: warning - write to MSB of TC0640FIO address %02x\n",cpu_get_pc(space->cpu),offset);
	}
}

READ16_DEVICE_HANDLER( tc0640fio_halfword_byteswap_r )
{
	return tc0640fio_halfword_r(device, offset, mem_mask) << 8;
}

WRITE16_DEVICE_HANDLER( tc0640fio_halfword_byteswap_w )
{
	if (ACCESSING_BITS_8_15)
		tc0640fio_w(device, offset, (data >> 8) & 0xff);
	else
	{
		tc0640fio_w(device, offset, data & 0xff);
//logerror("CPU #0 PC %06x: warning - write to LSB of TC0640FIO address %02x\n",cpu_get_pc(space->cpu),offset);
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0640fio )
{
	tc0640fio_state *tc0640fio =  tc0640fio_get_safe_token(device);
	const tc0640fio_interface *intf = tc0640fio_get_interface(device);

	devcb_resolve_read8(&tc0640fio->read_0, &intf->read_0, device);
	devcb_resolve_read8(&tc0640fio->read_1, &intf->read_1, device);
	devcb_resolve_read8(&tc0640fio->read_2, &intf->read_2, device);
	devcb_resolve_read8(&tc0640fio->read_3, &intf->read_3, device);
	devcb_resolve_read8(&tc0640fio->read_7, &intf->read_7, device);

	state_save_register_device_item_array(device, 0, tc0640fio->regs);
}

static DEVICE_RESET( tc0640fio )
{
	tc0640fio_state *tc0640fio =  tc0640fio_get_safe_token(device);
	int i;

	for (i = 0; i < 8; i++)
		tc0640fio->regs[i] = 0;
}

DEVICE_GET_INFO( tc0220ioc )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(tc0220ioc_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(tc0220ioc);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(tc0220ioc);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito TC0220IOC");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito I/O");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( tc0510nio )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(tc0510nio_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(tc0510nio);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(tc0510nio);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito TC0510NIO");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito I/O");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( tc0640fio )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(tc0640fio_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(tc0640fio);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(tc0640fio);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito TC0640FIO");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito I/O");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEFINE_LEGACY_DEVICE(TC0220IOC, tc0220ioc);
DEFINE_LEGACY_DEVICE(TC0510NIO, tc0510nio);
DEFINE_LEGACY_DEVICE(TC0640FIO, tc0640fio);
