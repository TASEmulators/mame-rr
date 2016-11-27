/*
    Generate the tms9900 emulator
*/

#include "emu.h"
#include "debugger.h"
#include "tms9900.h"

#define TMS99XX_MODEL TMS9900_ID

#include "99xxcore.h"

DEFINE_LEGACY_CPU_DEVICE(TMS9900, tms9900);
