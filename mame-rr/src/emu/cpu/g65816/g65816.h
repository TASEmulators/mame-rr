#pragma once

#ifndef __G65816_H__
#define __G65816_H__

#include "g65816cm.h"

/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

G65C816 CPU Emulator V0.92

Copyright Karl Stenerud
All rights reserved.

Permission is granted to use this source code for non-commercial purposes.
To use this code for commercial purposes, you must get permission from the
author (Karl Stenerud) at karl@higashiyama-unet.ocn.ne.jp.


*/
/* ======================================================================== */
/* ============================= Configuration ============================ */
/* ======================================================================== */

/* GTE Microcircuits G65816 */

/* ======================================================================== */
/* =============================== DEFINES ================================ */
/* ======================================================================== */

/* Interrupt lines - used with g65816_set_irq_line() */
enum
{
	G65816_LINE_NONE,
	G65816_LINE_IRQ,
	G65816_LINE_NMI,
	G65816_LINE_ABORT,
	G65816_LINE_SO,
	G65816_LINE_RDY,
	G65816_LINE_RESET
};

#define G65816_INT_NONE G65816_LINE_NONE
#define G65816_INT_IRQ G65816_LINE_IRQ
#define G65816_INT_NMI G65816_LINE_NMI


/* Registers - used by g65816_set_reg() and g65816_get_reg() */
enum
{
	G65816_PC=1, G65816_S, G65816_P, G65816_A, G65816_X, G65816_Y,
	G65816_PB, G65816_DB, G65816_D, G65816_E,
	G65816_NMI_STATE, G65816_IRQ_STATE
};

/* Main interface function */
DECLARE_LEGACY_CPU_DEVICE(G65816, g65816);
DECLARE_LEGACY_CPU_DEVICE(_5A22, _5a22);

#define CPU_TYPE_G65816 0
#define CPU_TYPE_5A22 1


void g65816_set_read_vector_callback(running_device *device, read8_space_func read_vector);


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* __G65816_H__ */
