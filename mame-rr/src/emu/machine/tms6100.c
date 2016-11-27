/**********************************************************************************************

     TMS6100 simulator

     Written for MAME by Couriersud

     Todo:
        - implement CS
        - implement 4 bit mode (mask programmed)
        - implement chip addressing (0-15 mask programmed)

     TMS6100:

                 +-----------------+
       VDD       |  1           28 |  NC
       NC        |  2           27 |  NC
       DATA/ADD1 |  3           26 |  NC
       DATA/ADD2 |  4           25 |  NC
       DATA/ADD4 |  5           24 |  NC
       DATA/ADD8 |  6           23 |  NC
       CLK       |  7           22 |  NC
       NC        |  8           21 |  NC
       NC        |  9           20 |  NC
       M0        | 10           19 |  NC
       M1        | 11           18 |  NC
       NC        | 12           17 |  NC
       /CS       | 13           16 |  NC
       VSS       | 14           15 |  NC
                 +-----------------+

    M58819 (from radarscope schematics):

                 +-----------------+
       AD0       |  1           40 |  AD1
       GND       |  2           39 |  AD2
       -5V       |  3           38 |  AD3
       A0        |  4           37 |  AD4
       NC        |  5           36 |  AD5
       NC        |  6           35 |  AD6
       A1        |  7           34 |  AD7
       A2        |  8           33 |  AD8
       A3        |  9           32 |  AD9
       CLK       | 10           31 |  AD10
       NC        | 11           30 |  AD11
       -5V       | 12           29 |  AD12
       C0        | 13           28 |  NC
       C1        | 14           27 |  NC
       NC        | 15           26 |  I7
       NC        | 16           25 |  NC
       +5V       | 17           24 |  I6
       I0        | 18           23 |  I5
       I1        | 19           22 |  I4
       I2        | 20           21 |  I3
                 +-----------------+

    The M58819 is used as an interface to external speech eproms.
    NC pins may have a function, although they are not connected in
    radarscope.

***********************************************************************************************/

#include "emu.h"
#include "tms6100.h"

#define VERBOSE		(0)

#if	VERBOSE
#define LOG(x)		logerror x
#else
#define	LOG(x)
#endif

#define TMS6100_READ_PENDING		0x01
#define TMS6100_NEXT_READ_IS_DUMMY	0x02

typedef struct _tms6100_state tms6100_state;
struct _tms6100_state
{
	/* Rom interface */
	UINT32 address;
	UINT32 address_latch;
	UINT8  loadptr;
	UINT8  m0;
	UINT8  m1;
	UINT8  addr_bits;
	UINT8  clock;
	UINT8  data;
	UINT8  state;

	const UINT8 *rom;

	running_device *device;

#if 0
	const tms5110_interface *intf;
#endif
};

/**********************************************************************************************

     get_safe_token -- get tms6100_state

***********************************************************************************************/

INLINE tms6100_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == TMS6100 ||
			device->type() == M58819);
	return (tms6100_state *)downcast<legacy_device_base *>(device)->token();
}

/**********************************************************************************************

     register_save_states -- register state vars for saving

***********************************************************************************************/

static void register_for_save_states(tms6100_state *tms)
{
	state_save_register_device_item(tms->device, 0, tms->addr_bits);
	state_save_register_device_item(tms->device, 0, tms->address);
	state_save_register_device_item(tms->device, 0, tms->address_latch);
	state_save_register_device_item(tms->device, 0, tms->data);
	state_save_register_device_item(tms->device, 0, tms->loadptr);
	state_save_register_device_item(tms->device, 0, tms->m0);
	state_save_register_device_item(tms->device, 0, tms->m1);
	state_save_register_device_item(tms->device, 0, tms->state);
}

/******************************************************************************

     DEVICE_START( tms6100 ) -- allocate buffers and reset the 6100

******************************************************************************/

static DEVICE_START( tms6100 )
{
	//static const tms5110_interface dummy = { 0 };
	tms6100_state *tms = get_safe_token(device);

	assert_always(tms != NULL, "Error creating TMS6100 chip");

	//tms->intf = device->baseconfig().static_config ? (const tms5110_interface *)device->baseconfig().static_config : &dummy;
	tms->rom = *device->region();

	tms->device = device;

	register_for_save_states(tms);
}

static DEVICE_START( m58819 )
{
	//tms6100_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms6100 );
	//tms5110_set_variant(tms, TMS5110_IS_5100);
}

static DEVICE_RESET( tms6100 )
{
	tms6100_state *tms = get_safe_token(device);

	/* initialize the chip */
	tms->addr_bits = 0;
	tms->address = 0;
	tms->address_latch = 0;
	tms->loadptr = 0;
	tms->m0 = 0;
	tms->m1 = 0;
	tms->state = 0;
	tms->clock = 0;
	tms->data = 0;
}

WRITE_LINE_DEVICE_HANDLER( tms6100_m0_w )
{
	tms6100_state *tms = get_safe_token(device);
	if (state != tms->m0)
		tms->m0 = state;
}

WRITE_LINE_DEVICE_HANDLER( tms6100_m1_w )
{
	tms6100_state *tms = get_safe_token(device);
	if (state != tms->m1)
		tms->m1 = state;
}

WRITE_LINE_DEVICE_HANDLER( tms6100_romclock_w )
{
	tms6100_state *tms = get_safe_token(device);

	/* process on falling edge */
	if (tms->clock && !state)
	{
		switch ((tms->m1<<1) | tms->m0)
		{
		case 0x00:
			/* NOP in datasheet, not really ... */
			if (tms->state & TMS6100_READ_PENDING)
			{
				if (tms->state & TMS6100_NEXT_READ_IS_DUMMY)
				{
					tms->address = (tms->address_latch << 3);
					tms->address_latch = 0;
					tms->loadptr = 0;
					tms->state &= ~TMS6100_NEXT_READ_IS_DUMMY;
					LOG(("loaded address %08x\n", tms->address));
				}
				else
				{
					/* read bit at address */
					tms->data = (tms->rom[tms->address >> 3] >> ((tms->address & 0x07) ^ 0x07)) & 1;
					tms->address++;
				}
				tms->state &= ~TMS6100_READ_PENDING;
			}
			break;
		case 0x01:
			/* READ */
			tms->state |= TMS6100_READ_PENDING;
			break;
		case 0x02:
			/* LOAD ADDRESS */
			tms->state |= TMS6100_NEXT_READ_IS_DUMMY;
			tms->address_latch |= (tms->addr_bits << tms->loadptr);
			LOG(("loaded address latch %08x\n", tms->address_latch));
			tms->loadptr += 4;
			break;
		case 0x03:
			/* READ AND BRANCH */
			if (tms->state & TMS6100_NEXT_READ_IS_DUMMY)
			{
				tms->state &= ~TMS6100_NEXT_READ_IS_DUMMY;  // clear - no dummy read according to datasheet
				LOG(("loaded address latch %08x\n", tms->address_latch));
				tms->address = tms->rom[tms->address_latch] | (tms->rom[tms->address_latch+1]<<8);
				tms->address &= 0x3fff; // 14 bits
				LOG(("loaded indirect address %04x\n", tms->address));
				tms->address = (tms->address << 3);
				tms->address_latch = 0;
				tms->loadptr = 0;
			}
			break;
		}
	}
	tms->clock = state;
}

WRITE8_DEVICE_HANDLER( tms6100_addr_w )
{
	tms6100_state *tms = get_safe_token(device);
	if (data != tms->addr_bits)
		tms->addr_bits = data;
}

READ_LINE_DEVICE_HANDLER( tms6100_data_r )
{
	tms6100_state *tms = get_safe_token(device);

	return tms->data;
}

/*-------------------------------------------------
    TMS 6100 device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)				p##tms6100##s
#define DEVTEMPLATE_FEATURES			DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME				"TMS6100"
#define DEVTEMPLATE_FAMILY				"TI Speech"
#include "devtempl.h"

#define DEVTEMPLATE_DERIVED_ID(p,s)		p##m58819##s
#define DEVTEMPLATE_DERIVED_FEATURES	DT_HAS_START
#define DEVTEMPLATE_DERIVED_NAME		"M58819"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(TMS6100, tms6100);
DEFINE_LEGACY_DEVICE(M58819, m58819);
