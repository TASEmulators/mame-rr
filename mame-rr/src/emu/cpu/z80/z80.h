#pragma once

#ifndef __Z80_H__
#define __Z80_H__

enum
{
	NSC800_RSTA = 1,
	NSC800_RSTB,
	NSC800_RSTC
};

enum
{
	Z80_PC, Z80_SP,
	Z80_A, Z80_B, Z80_C, Z80_D, Z80_E, Z80_H, Z80_L,
	Z80_AF, Z80_BC, Z80_DE, Z80_HL,
	Z80_IX, Z80_IY,	Z80_AF2, Z80_BC2, Z80_DE2, Z80_HL2,
	Z80_R, Z80_I, Z80_IM, Z80_IFF1, Z80_IFF2, Z80_HALT,
	Z80_DC0, Z80_DC1, Z80_DC2, Z80_DC3, Z80_WZ,

	Z80_GENPC = STATE_GENPC,
	Z80_GENSP = STATE_GENSP,
	Z80_GENPCBASE = STATE_GENPCBASE
};

DECLARE_LEGACY_CPU_DEVICE(Z80, z80);
DECLARE_LEGACY_CPU_DEVICE(NSC800, nsc800);

CPU_DISASSEMBLE( z80 );

void z80_set_cycle_tables(running_device *device, const UINT8 *op, const UINT8 *cb, const UINT8 *ed, const UINT8 *xy, const UINT8 *xycb, const UINT8 *ex);

#endif /* __Z80_H__ */
