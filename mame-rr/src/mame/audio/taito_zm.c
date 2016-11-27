/***************************************************************************

    Taito Zoom ZSG-2 sound board
    Includes: MN10200 CPU, ZOOM ZSG-2 audio chip, TMS57002 DASP
    By Olivier Galibert.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "cpu/mn10200/mn10200.h"
#include "cpu/tms57002/tms57002.h"
#include "audio/taito_zm.h"
#include "sound/zsg2.h"

static ADDRESS_MAP_START(taitozoom_map, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("mn10200", 0)
	AM_RANGE(0x400000, 0x40ffff) AM_RAM
	AM_RANGE(0x800000, 0x800fff) AM_DEVREADWRITE("zsg2", zsg2_r, zsg2_w)
	AM_RANGE(0xe00000, 0xe000ff) AM_RAM	// main CPU comms (1fbe0xxx on FX-1B main CPU, banked with eeprom - raystorm writes command at PC=80015240)
	AM_RANGE(0xc00000, 0xc00001) AM_RAM	// TMS57002 comms
ADDRESS_MAP_END

static UINT8 tms_ctrl;

static READ8_HANDLER(tms_ctrl_r)
{
	return tms_ctrl;
}

static WRITE8_HANDLER(tms_ctrl_w)
{
#if 0
	tms57002_reset_w(data & 4);
	tms57002_cload_w(data & 2);
	tms57002_pload_w(data & 1);
#endif

	tms_ctrl = data;
}

static ADDRESS_MAP_START(taitozoom_io_map, ADDRESS_SPACE_IO, 8)
	AM_RANGE(MN10200_PORT1, MN10200_PORT1) AM_READWRITE(tms_ctrl_r, tms_ctrl_w)
ADDRESS_MAP_END

static const zsg2_interface zsg2_taito_config =
{
	"zsg2"	/* sample region */
};

MACHINE_DRIVER_START( taito_zoom_sound )
	MDRV_CPU_ADD("mn10200", MN10200, 25000000/2)
	MDRV_CPU_PROGRAM_MAP(taitozoom_map)
	MDRV_CPU_IO_MAP(taitozoom_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_pulse)

	// we assume the parent machine has created lspeaker/rspeaker
	MDRV_SOUND_ADD("zsg2", ZSG2, 25000000/2)
	MDRV_SOUND_CONFIG(zsg2_taito_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END
