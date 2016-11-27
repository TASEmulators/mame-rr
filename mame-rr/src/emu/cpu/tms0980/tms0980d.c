/*******************************************************************

tms0980d.c
TMS0980 disassembly

*******************************************************************/

#include "emu.h"
#include "debugger.h"
#include "tms0980.h"


#define _OVER DASMFLAG_STEP_OVER
#define _OUT  DASMFLAG_STEP_OUT


enum e_mnemonics {
	zA10AAC=0, zA6AAC, zA8AAC, zAC1AC, zACACC, zACNAA, zALEC, zALEM, zAMAAC, zBRANCH, zCALL, zCCLA,
	zCLA, zCLO, zCOMC, zCOMX, zCOMX8, zCPAIZ, zCTMDYN, zDAN, zDMAN, zDMEA, zDNAA,
	zDYN, zIA, zIMAC, zIYC, zKNE, zKNEZ, zLDP, zLDX, zLDX4, zMNEA, zMNEZ,
	zNDMEA, zOFF, zRBIT, zREAC, zRETN, zRSTR, zSAL, zSAMAN, zSBIT,
	zSBL, zSEAC, zSETR, zTAM, zTAMACS, zTAMDYN, zTAMIY, zTAMIYC, zTAMZA,
	zTAY, zTBIT, zTCMIY, zTCY, zTDO, zTKA, zTKM, zTMA,
	zTMY, zTYA, zXDA, zXMA, zYMCY, zYNEA, zYNEC,
	zILL
};


enum e_addressing {
	zB0=0, zB2, zI2, zI4, zB7
};


static const char *const s_mnemonic[] = {
	"a10aac", "a6aac", "a8aac", "ac1ac", "acacc", "acnaa", "alec", "alem", "amaac", "branch", "call", "ccla",
	"cla", "clo", "comc", "comx", "comx8", "cpaiz", "ctmdyn", "dan", "dman", "dmea", "dnaa",
	"dyn", "ia", "imac", "iyc", "kne", "knez", "ldp", "ldx", "ldx", "mnea", "mnez",
	"ndmea", "off", "rbit", "reac", "retn", "rstr", "sal", "saman", "sbit",
	"sbl", "seac", "setr", "tam", "tamacs", "tamdyn", "tamiy", "tamiyc", "tamza",
	"tay", "tbit", "tcmiy", "tcy", "tdo", "tka", "tkm", "tma",
	"tmy", "tya", "xda", "xma", "ymcy", "ynea", "ynec",
	"illegal"
};


static const UINT32 s_flags[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  _OVER, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, _OUT, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0
};


static const UINT8 s_addressing[] = {
	zB0, zB0, zB0, zI4, zI4, zI4, zI4, zB0, zB0, zB7, zB7, zB0,
	zB0, zB0, zB0, zB0, zB0, zB0, zB0, zB0, zB0, zB0, zB0,
	zB0, zB0, zB0, zB0, zB0, zB0, zI4, zI2, zI4, zB0, zB0,
	zB0, zB0, zB0, zB0, zB0, zB0, zB0, zB0, zB0,
	zB0, zB0, zB0, zB0, zI4, zB0, zB0, zB0, zB2,
	zB0, zB2, zI4, zI4, zB0, zB0, zB0, zB0,
	zB0, zB0, zB0, zB0, zI4, zB0, zI4,
	zB0
};


static const UINT8 tms0980_mnemonic[512] = {
	/* 0x000 */
	zCOMX, zALEM, zYNEA, zXMA, zDYN, zIYC, zCLA, zDMAN,
	zTKA, zMNEA, zTKM, zILL, zILL, zSETR, zKNE, zILL,
	/* 0x010 */
	zDMEA, zDNAA, zCCLA, zNDMEA, zILL, zAMAAC, zILL, zILL,
	zCTMDYN, zXDA, zILL, zILL, zILL, zILL, zILL, zILL,
	/* 0x020 */
	zTBIT, zTBIT, zTBIT, zTBIT, zILL, zILL, zILL, zILL,
	zTAY, zTMA, zTMY, zTYA, zTAMDYN, zTAMIYC, zTAMZA, zTAM,
	/* 0x030 */
	zSAMAN, zCPAIZ, zIMAC, zMNEZ, zILL, zILL, zILL, zILL,
	zTCY, zYNEC, zTCMIY, zACACC, zACNAA, zTAMACS, zALEC, zYMCY,
	/* 0x040 */
	zTCY, zTCY, zTCY, zTCY, zTCY, zTCY, zTCY, zTCY,
	zTCY, zTCY, zTCY, zTCY, zTCY, zTCY, zTCY, zTCY,
	/* 0x050 */
	zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC,
	zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC,
	/* 0x060 */
	zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY,
	zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY,
	/* 0x070 */
	zACACC, zACACC, zACACC, zACACC, zACACC, zACACC, zACACC, zACACC,
	zACACC, zACACC, zACACC, zACACC, zACACC, zACACC, zACACC, zACACC,
	/* 0x080 */
	zLDP, zLDP, zLDP, zLDP, zLDP, zLDP, zLDP, zLDP,
	zLDP, zLDP, zLDP, zLDP, zLDP, zLDP, zLDP, zLDP,
	/* 0x090 */
	zLDX4, zLDX4, zLDX4, zLDX4, zLDX4, zLDX4, zLDX4, zLDX4,
	zLDX4, zLDX4, zLDX4, zLDX4, zLDX4, zLDX4, zLDX4, zLDX4,
	/* 0x0A0 */
	zSBIT, zSBIT, zSBIT, zSBIT, zRBIT, zRBIT, zRBIT, zRBIT,
	zILL, zILL, zILL, zILL, zILL, zILL, zILL, zILL,
	/* 0x0B0 */
	zTDO, zSAL, zCOMX8, zSBL, zREAC, zSEAC, zOFF, zILL,
	zILL, zILL, zILL, zILL, zILL, zILL, zILL, zRETN,
	/* 0x0C0 */
	zACNAA, zACNAA, zACNAA, zACNAA, zACNAA, zACNAA, zACNAA, zACNAA,
	zACNAA, zACNAA, zACNAA, zACNAA, zACNAA, zACNAA, zACNAA, zACNAA,
	/* 0x0D0 */
	zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS,
	zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS,
	/* 0x0E0 */
	zALEC, zALEC, zALEC, zALEC, zALEC, zALEC, zALEC, zALEC,
	zALEC, zALEC, zALEC, zALEC, zALEC, zALEC, zALEC, zALEC,
	/* 0x0F0 */
	zYMCY, zYMCY, zYMCY, zYMCY, zYMCY, zYMCY, zYMCY, zYMCY,
	zYMCY, zYMCY, zYMCY, zYMCY, zYMCY, zYMCY, zYMCY, zYMCY,
	/* 0x100 */
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	/* 0x180 */
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL
};


static const UINT8 tms1000_mnemonic[256] = {
	/* 0x00 */
	zCOMX, zA8AAC, zYNEA, zTAM, zTAMZA, zA10AAC, zA6AAC, zDAN,
	zTKA, zKNEZ, zTDO, zCLO, zRSTR, zSETR, zIA, zRETN,
	zLDP, zLDP, zLDP, zLDP, zLDP, zLDP, zLDP, zLDP,
	zLDP, zLDP, zLDP, zLDP, zLDP, zLDP, zLDP, zLDP,
	zTAMIY, zTMA, zTMY, zTYA, zTAY, zAMAAC, zMNEZ, zSAMAN,
	zIMAC, zALEM, zDMAN, zIYC, zDYN, zCPAIZ, zXMA, zCLA,
	zSBIT, zSBIT, zSBIT, zSBIT, zRBIT, zRBIT, zRBIT, zRBIT,
	zTBIT, zTBIT, zTBIT, zTBIT, zLDX, zLDX, zLDX, zLDX,
	/* 0x40 */
	zTCY, zTCY, zTCY, zTCY, zTCY, zTCY, zTCY, zTCY,
	zTCY, zTCY, zTCY, zTCY, zTCY, zTCY, zTCY, zTCY,
	zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC,
	zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC,
	zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY,
	zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY,
	zALEC, zALEC, zALEC, zALEC, zALEC, zALEC, zALEC, zALEC,
	zALEC, zALEC, zALEC, zALEC, zALEC, zALEC, zALEC, zALEC,
	/* 0x80 */
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	/* 0xc0 */
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL
};


static const UINT8 tms1100_mnemonic[256] = {
	/* 0x00 */
	zMNEA, zALEM, zYNEA, zXMA, zDYN, zIYC, zAMAAC, zDMAN,
	zTKA, zCOMX, zTDO, zCOMC, zRSTR, zSETR, zKNEZ, zRETN,
	zLDP, zLDP, zLDP, zLDP, zLDP, zLDP, zLDP, zLDP,
	zLDP, zLDP, zLDP, zLDP, zLDP, zLDP, zLDP, zLDP,
	/* 0x20 */
	zTAY, zTMA, zTMY, zTYA, zTAMDYN, zTAMIYC, zTAMZA, zTAM,
	zLDX, zLDX, zLDX, zLDX, zLDX, zLDX, zLDX, zLDX,
	zSBIT, zSBIT, zSBIT, zSBIT, zRBIT, zRBIT, zRBIT, zRBIT,
	zTBIT, zTBIT, zTBIT, zTBIT, zSAMAN, zCPAIZ, zIMAC, zMNEZ,
	/* 0x40 */
	zTCY, zTCY, zTCY, zTCY, zTCY, zTCY, zTCY, zTCY,
	zTCY, zTCY, zTCY, zTCY, zTCY, zTCY, zTCY, zTCY,
	zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC,
	zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC, zYNEC,
	/* 0x60 */
	zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY,
	zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY, zTCMIY,
	zAC1AC, zAC1AC, zAC1AC, zAC1AC, zAC1AC, zAC1AC, zAC1AC, zAC1AC,
	zAC1AC, zAC1AC, zAC1AC, zAC1AC, zAC1AC, zAC1AC, zAC1AC, zCLA,
	/* 0x80 */
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH,
	/* 0xC0 */
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
	zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL, zCALL,
};

static const UINT8 tms0980_i2_value[4] =
{
	0x00, 0x02, 0x01, 0x03
};
static const UINT8 tms0980_i4_value[16] =
{
	0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09, 0x05, 0x0D, 0x03, 0x0B, 0x07, 0x0F
};
static const UINT8 tms0980_bit_value[4] = { 1, 4, 2, 8 };


CPU_DISASSEMBLE( tms0980 ) {
	char *dst = buffer;
	UINT16 op, instr;
	int pos = 0;

	op = ( ( oprom[pos] << 8 ) | oprom[pos + 1]  ) & 0x01FF;
	pos += 2;

	instr = tms0980_mnemonic[op];

	dst += sprintf( dst, "%-8s ", s_mnemonic[instr] );

	switch( s_addressing[instr] ) {
	case zB0:
		break;
	case zB2:
		dst += sprintf( dst, "#$%d", tms0980_bit_value[ op & 3 ] );
		break;
	case zI2:
		dst += sprintf( dst, "#$%01X", tms0980_i2_value[ op & 0x03 ] );
		break;
	case zI4:
		dst += sprintf( dst, "#$%01X", tms0980_i4_value[ op & 0x0F ] );
		break;
	case zB7:
		dst += sprintf( dst, "#$%02X", ( op & 0x7F ) << 1 );
		break;
	}

	return pos | s_flags[instr] | DASMFLAG_SUPPORTED;
}


CPU_DISASSEMBLE( tms1000 ) {
	char *dst = buffer;
	UINT8 op, instr;
	int pos = 0;

	op = oprom[pos];
	pos += 1;

	instr = tms1000_mnemonic[op];

	dst += sprintf( dst, "%-8s ", s_mnemonic[instr] );

	switch( s_addressing[instr] ) {
	case zB0:
		break;
	case zB2:
		dst += sprintf( dst, "#$%d", tms0980_bit_value[ op & 3 ] );
		break;
	case zI2:
		dst += sprintf( dst, "#$%01X", tms0980_i2_value[ op & 0x03 ] );
		break;
	case zI4:
		dst += sprintf( dst, "#$%01X", tms0980_i4_value[ op & 0x0F ] );
		break;
	case zB7:
		dst += sprintf( dst, "#$%02X", ( op & 0x3F ) );
		break;
	}

	return pos | s_flags[instr] | DASMFLAG_SUPPORTED;
}


CPU_DISASSEMBLE( tms1100 ) {
	char *dst = buffer;
	UINT8 op, instr;
	int pos = 0;

	op = oprom[pos];
	pos += 1;

	instr = tms1100_mnemonic[op];

	dst += sprintf( dst, "%-8s ", s_mnemonic[instr] );

	switch( s_addressing[instr] ) {
	case zB0:
		break;
	case zB2:
		dst += sprintf( dst, "#$%d", tms0980_bit_value[ op & 3 ] );
		break;
	case zI2:
		dst += sprintf( dst, "#$%01X", tms0980_i2_value[ op & 0x03 ] );
		break;
	case zI4:
		dst += sprintf( dst, "#$%01X", tms0980_i4_value[ op & 0x0F ] );
		break;
	case zB7:
		dst += sprintf( dst, "#$%02X", ( op & 0x3F ) );
		break;
	}

	return pos | s_flags[instr] | DASMFLAG_SUPPORTED;
}

