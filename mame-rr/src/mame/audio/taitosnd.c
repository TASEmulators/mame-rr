#include "emu.h"
#include "cpu/z80/z80.h"
#include "taitosnd.h"


/**********************************************************************************************

    It seems like 1 nibble commands are only for control purposes.
    2 nibble commands are the real messages passed from one board to the other.

**********************************************************************************************/

/* Some logging defines */
#if 0
#define REPORT_SLAVE_MODE_CHANGE
#define REPORT_SLAVE_MODE_READ_ITSELF
#define REPORT_MAIN_MODE_READ_SLAVE
#define REPORT_DATA_FLOW
#endif

#define TC0140SYT_PORT01_FULL         (0x01)
#define TC0140SYT_PORT23_FULL         (0x02)
#define TC0140SYT_PORT01_FULL_MASTER  (0x04)
#define TC0140SYT_PORT23_FULL_MASTER  (0x08)

typedef struct _tc0140syt_state tc0140syt_state;
struct _tc0140syt_state
{
	UINT8     slavedata[4];  /* Data on master->slave port (4 nibbles) */
	UINT8     masterdata[4]; /* Data on slave->master port (4 nibbles) */
	UINT8     mainmode;      /* Access mode on master cpu side */
	UINT8     submode;       /* Access mode on slave cpu side */
	UINT8     status;        /* Status data */
	UINT8     nmi_enabled;   /* 1 if slave cpu has nmi's enabled */
	UINT8     nmi_req;       /* 1 if slave cpu has a pending nmi */

	running_device *mastercpu;	/* this is the maincpu */
	running_device *slavecpu;	/* this is the audiocpu */
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0140syt_state *get_safe_token( running_device *device )
{
	assert(device != NULL);
	assert(device->type() == TC0140SYT);

	return (tc0140syt_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const tc0140syt_interface *get_interface( running_device *device )
{
	assert(device != NULL);
	assert((device->type() == TC0140SYT));
	return (const tc0140syt_interface *) device->baseconfig().static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

static void interrupt_controller( running_device *device )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);

	if (tc0140syt->nmi_req && tc0140syt->nmi_enabled)
	{
		cpu_set_input_line(tc0140syt->slavecpu, INPUT_LINE_NMI, PULSE_LINE);
		tc0140syt->nmi_req = 0;
	}
}

WRITE8_DEVICE_HANDLER( tc0140syt_port_w )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);
	data &= 0x0f;

	tc0140syt->mainmode = data;
	//logerror("taitosnd: Master cpu mode [%02x]\n", data);
	if (data > 4)
	{
		logerror("tc0140syt : error Master entering unknown mode[%02x]\n", data);
	}
}

WRITE8_DEVICE_HANDLER( tc0140syt_comm_w )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);

	data &= 0x0f;	/*this is important, otherwise ballbros won't work*/

	switch (tc0140syt->mainmode)
	{
		case 0x00:		// mode #0
			tc0140syt->slavedata[tc0140syt->mainmode ++] = data;
			//logerror("taitosnd: Master cpu written port 0, data %01x\n", data);
			break;

		case 0x01:		// mode #1
			tc0140syt->slavedata[tc0140syt->mainmode ++] = data;
			tc0140syt->status |= TC0140SYT_PORT01_FULL;
			tc0140syt->nmi_req = 1;
			//logerror("taitosnd: Master cpu sends 0/1 : %01x%01x\n", tc0140syt->slavedata[1], tc0140syt->slavedata[0]);
        	break;

		case 0x02:		// mode #2
			tc0140syt->slavedata[tc0140syt->mainmode ++] = data;
			//logerror("taitosnd: Master cpu written port 2, data %01\n", data);
			break;

		case 0x03:		// mode #3
			tc0140syt->slavedata[tc0140syt->mainmode ++] = data;
			tc0140syt->status |= TC0140SYT_PORT23_FULL;
			tc0140syt->nmi_req = 1;
			//logerror("taitosnd: Master cpu sends 2/3 : %01x%01x\n", tc0140syt->slavedata[3], tc0140syt->slavedata[2]);
			break;

		case 0x04:		// port status
//#ifdef REPORT_DATA_FLOW
			//logerror("taitosnd: Master issued control value %02x (PC = %08x) \n",data, cpu_get_pc(space->cpu) );
//#endif
			/* this does a hi-lo transition to reset the sound cpu */
			if (data)
				cpu_set_input_line(tc0140syt->slavecpu, INPUT_LINE_RESET, ASSERT_LINE);
			else
			{
				cpu_set_input_line(tc0140syt->slavecpu, INPUT_LINE_RESET, CLEAR_LINE);
				cpu_spin(tc0140syt->mastercpu); /* otherwise no sound in driftout */
			}
			break;

		default:
			logerror("taitosnd: Master cpu written in mode [%02x] data[%02x]\n", tc0140syt->mainmode, data);
	}

}

READ8_DEVICE_HANDLER( tc0140syt_comm_r )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);

	switch( tc0140syt->mainmode )
	{
		case 0x00:		// mode #0
			//logerror("taitosnd: Master cpu read portdata %01x\n", tc0140syt->masterdata[0]);
			return tc0140syt->masterdata[tc0140syt->mainmode ++];

		case 0x01:		// mode #1
			//logerror("taitosnd: Master cpu receives 0/1 : %01x%01x\n", tc0140syt->masterdata[1], tc0140syt->masterdata[0]);
			tc0140syt->status &= ~TC0140SYT_PORT01_FULL_MASTER;
			return tc0140syt->masterdata[tc0140syt->mainmode ++];

		case 0x02:		// mode #2
			//logerror("taitosnd: Master cpu read masterdata %01x\n", tc0140syt->masterdata[2]);
			return tc0140syt->masterdata[tc0140syt->mainmode ++];

		case 0x03:		// mode #3
			//logerror("taitosnd: Master cpu receives 2/3 : %01x%01x\n", tc0140syt->masterdata[3], tc0140syt->masterdata[2]);
			tc0140syt->status &= ~TC0140SYT_PORT23_FULL_MASTER;
			return tc0140syt->masterdata[tc0140syt->mainmode ++];

		case 0x04:		// port status
			//logerror("tc0140syt : Master cpu read status : %02x\n", tc0140syt->status);
			return tc0140syt->status;

		default:
			logerror("tc0140syt : Master cpu read in mode [%02x]\n", tc0140syt->mainmode);
			return 0;
	}
}

//SLAVE SIDE

WRITE8_DEVICE_HANDLER( tc0140syt_slave_port_w )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);

	data &= 0x0f;
	tc0140syt->submode = data;
	//logerror("taitosnd: Slave cpu mode [%02x]\n", data);
	if (data > 6)
		logerror("tc0140syt error : Slave cpu unknown mode[%02x]\n", data);
}

WRITE8_DEVICE_HANDLER( tc0140syt_slave_comm_w )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);

	data &= 0x0f;
	switch (tc0140syt->submode)
	{
		case 0x00:		// mode #0
			tc0140syt->masterdata[tc0140syt->submode ++] = data;
			//logerror("taitosnd: Slave cpu written port 0, data %01x\n", data);
			break;

		case 0x01:		// mode #1
			tc0140syt->masterdata[tc0140syt->submode ++] = data;
			tc0140syt->status |= TC0140SYT_PORT01_FULL_MASTER;
			//logerror("taitosnd: Slave cpu sends 0/1 : %01x%01x\n" , tc0140syt->masterdata[1] , tc0140syt->masterdata[0]);
			cpu_spin(tc0140syt->slavecpu); /* writing should take longer than emulated, so spin */
			break;

		case 0x02:		// mode #2
			//logerror("taitosnd: Slave cpu written port 2, data %01x\n", data);
			tc0140syt->masterdata[tc0140syt->submode ++] = data;
			break;

		case 0x03:		// mode #3
			tc0140syt->masterdata[tc0140syt->submode ++] = data;
			tc0140syt->status |= TC0140SYT_PORT23_FULL_MASTER;
			//logerror("taitosnd: Slave cpu sends 2/3 : %01x%01x\n" , tc0140syt->masterdata[3] , tc0140syt->masterdata[2]);
			cpu_spin(tc0140syt->slavecpu); /* writing should take longer than emulated, so spin */
			break;

		case 0x04:		// port status
			//tc0140syt->status = TC0140SYT_SET_OK;
			//logerror("tc0140syt : Slave cpu status ok.\n");
			break;

		case 0x05:		// nmi disable
			tc0140syt->nmi_enabled = 0;
			break;

		case 0x06:		// nmi enable
			tc0140syt->nmi_enabled = 1;
			break;

		default:
			logerror("tc0140syt: Slave cpu written in mode [%02x] data[%02x]\n" , tc0140syt->submode, data & 0xff);
	}

	interrupt_controller(device);

}

READ8_DEVICE_HANDLER( tc0140syt_slave_comm_r )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);
	UINT8 res = 0;

	switch ( tc0140syt->submode )
	{
		case 0x00:		// mode #0
			//logerror("taitosnd: Slave cpu read slavedata %01x\n", tc0140syt->slavedata[0]);
			res = tc0140syt->slavedata[tc0140syt->submode ++];
			break;

		case 0x01:		// mode #1
			//logerror("taitosnd: Slave cpu receives 0/1 : %01x%01x PC=%4x\n", tc0140syt->slavedata[1] , tc0140syt->slavedata[0],cpu_get_pc(space->cpu));
			tc0140syt->status &= ~TC0140SYT_PORT01_FULL;
			res = tc0140syt->slavedata[tc0140syt->submode ++];
			break;

		case 0x02:		// mode #2
			//logerror("taitosnd: Slave cpu read slavedata %01x\n", tc0140syt->slavedata[2]);
			res = tc0140syt->slavedata[tc0140syt->submode ++];
			break;

		case 0x03:		// mode #3
			//logerror("taitosnd: Slave cpu receives 2/3 : %01x%01x\n", tc0140syt->slavedata[3] , tc0140syt->slavedata[2]);
			tc0140syt->status &= ~TC0140SYT_PORT23_FULL;
			res = tc0140syt->slavedata[tc0140syt->submode ++];
			break;

		case 0x04:		// port status
			//logerror("tc0140syt : Slave cpu read status : %02x\n", tc0140syt->status);
			res = tc0140syt->status;
			break;

		default:
			logerror("tc0140syt : Slave cpu read in mode [%02x]\n", tc0140syt->submode);
			res = 0;
	}

	interrupt_controller(device);

	return res;
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0140syt )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);
	const tc0140syt_interface *intf = get_interface(device);

	/* use the given gfx set */
	tc0140syt->mastercpu = device->machine->device(intf->master);
	tc0140syt->slavecpu = device->machine->device(intf->slave);

	state_save_register_device_item(device, 0, tc0140syt->mainmode);
	state_save_register_device_item(device, 0, tc0140syt->submode);
	state_save_register_device_item(device, 0, tc0140syt->status);
	state_save_register_device_item(device, 0, tc0140syt->nmi_enabled);
	state_save_register_device_item(device, 0, tc0140syt->nmi_req);
	state_save_register_device_item_array(device, 0, tc0140syt->slavedata);
	state_save_register_device_item_array(device, 0, tc0140syt->masterdata);
}

static DEVICE_RESET( tc0140syt )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);
	int i;

	tc0140syt->mainmode = 0;
	tc0140syt->submode = 0;
	tc0140syt->status = 0;
	tc0140syt->nmi_enabled = 0;
	tc0140syt->nmi_req = 0;

	for (i = 0; i < 4; i++)
	{
		tc0140syt->slavedata[i] = 0;
		tc0140syt->masterdata[i] = 0;
	}
}

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##tc0140syt##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME		"Taito TC0140SYT"
#define DEVTEMPLATE_FAMILY		"Taito Audio Custom IC"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(TC0140SYT, tc0140syt);
