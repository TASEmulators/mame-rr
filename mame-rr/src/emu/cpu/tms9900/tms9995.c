/*
    generate the tms9995 emulator
*/

#include "emu.h"
#include "debugger.h"
#include "tms9900.h"

#define TMS99XX_MODEL TMS9995_ID

#include "99xxcore.h"

DEFINE_LEGACY_CPU_DEVICE(TMS9995, tms9995);
