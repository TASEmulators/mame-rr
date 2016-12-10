/*****************************************************************************
 *
 *   8085dasm.c
 *   Portable I8085A disassembler
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *   You can contact me at juergen@mame.net or pullmoll@stop1984.com
 *
 *   - This source code is released as freeware for non-commercial purposes
 *     as part of the M.A.M.E. (Multiple Arcade Machine Emulator) project.
 *     The licensing terms of MAME apply to this piece of code for the MAME
 *     project and derviative works, as defined by the MAME license. You
 *     may opt to make modifications, improvements or derivative works under
 *     that same conditions, and the MAME project may opt to keep
 *     modifications, improvements or derivatives under their terms exclusively.
 *
 *   - Alternatively you can choose to apply the terms of the "GPL" (see
 *     below) to this - and only this - piece of code or your derivative works.
 *     Note that in no case your choice can have any impact on any other
 *     source code of the MAME project, or binary, or executable, be it closely
 *     or losely related to this piece of code.
 *
 *  -  At your choice you are also free to remove either licensing terms from
 *     this file and continue to use it under only one of the two licenses. Do this
 *     if you think that licenses are not compatible (enough) for you, or if you
 *     consider either license 'too restrictive' or 'too free'.
 *
 *  -  GPL (GNU General Public License)
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License
 *     as published by the Free Software Foundation; either version 2
 *     of the License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *****************************************************************************/

#include "emu.h"

/* 8080/8085A mnemonics were more irritation than information
   What would you guess "CP $3456" to mean? It's not compare,
   but call if plus ... therefore: */
//#define Z80_MNEMONICS

#define OP(A)   oprom[(A) - PC]
#define ARG(A)  opram[(A) - PC]
#define ARGW(A) (opram[(A) - PC] | (opram[(A) + 1 - PC] << 8))

CPU_DISASSEMBLE( i8085 )
{
	UINT32 flags = 0;
	UINT8 op;
	unsigned PC = pc;
	switch (op = OP(pc++))
	{
#ifdef  Z80_MNEMONICS
		case 0x00: sprintf (buffer,"nop");                             break;
		case 0x01: sprintf (buffer,"ld   bc,$%04x", ARGW(pc)); pc+=2;  break;
		case 0x02: sprintf (buffer,"ld   (bc),a");                     break;
		case 0x03: sprintf (buffer,"inc  bc");                         break;
		case 0x04: sprintf (buffer,"inc  b");                          break;
		case 0x05: sprintf (buffer,"dec  b");                          break;
		case 0x06: sprintf (buffer,"ld   b,$%02x", ARG(pc)); pc++;     break;
		case 0x07: sprintf (buffer,"rlca");                            break;
		case 0x08: sprintf (buffer,"sub  hl,bc (*)");                  break;
		case 0x09: sprintf (buffer,"add  hl,bc");                      break;
		case 0x0a: sprintf (buffer,"ld   a,(bc)");                     break;
		case 0x0b: sprintf (buffer,"dec  bc");                         break;
		case 0x0c: sprintf (buffer,"inc  c");                          break;
		case 0x0d: sprintf (buffer,"dec  c");                          break;
		case 0x0e: sprintf (buffer,"ld   c,$%02x", ARG(pc)); pc++;     break;
		case 0x0f: sprintf (buffer,"rrca");                            break;
		case 0x10: sprintf (buffer,"sra  hl (*)");                     break;
		case 0x11: sprintf (buffer,"ld   de,$%04x", ARGW(pc)); pc+=2;  break;
		case 0x12: sprintf (buffer,"ld   (de),a");                     break;
		case 0x13: sprintf (buffer,"inc  de");                         break;
		case 0x14: sprintf (buffer,"inc  d");                          break;
		case 0x15: sprintf (buffer,"dec  d");                          break;
		case 0x16: sprintf (buffer,"ld   d,$%02x", ARG(pc)); pc++;     break;
		case 0x17: sprintf (buffer,"rla");                             break;
		case 0x18: sprintf (buffer,"rl   de (*)");                     break;
		case 0x19: sprintf (buffer,"add  hl,de");                      break;
		case 0x1a: sprintf (buffer,"ld   a,(de)");                     break;
		case 0x1b: sprintf (buffer,"dec  de");                         break;
		case 0x1c: sprintf (buffer,"inc  e");                          break;
		case 0x1d: sprintf (buffer,"dec  e");                          break;
		case 0x1e: sprintf (buffer,"ld   e,$%02x", ARG(pc)); pc++;     break;
		case 0x1f: sprintf (buffer,"rra");                             break;
		case 0x20: sprintf (buffer,"rim");                             break;
		case 0x21: sprintf (buffer,"ld   hl,$%04x", ARGW(pc)); pc+=2;  break;
		case 0x22: sprintf (buffer,"ld   ($%04x),hl", ARGW(pc)); pc+=2;break;
		case 0x23: sprintf (buffer,"inc  hl");                         break;
		case 0x24: sprintf (buffer,"inc  h");                          break;
		case 0x25: sprintf (buffer,"dec  h");                          break;
		case 0x26: sprintf (buffer,"ld   h,$%02x", ARG(pc)); pc++;     break;
		case 0x27: sprintf (buffer,"daa");                             break;
		case 0x28: sprintf (buffer,"ld   de,hl+$%02x (*)",ARG(pc));pc++;break;
		case 0x29: sprintf (buffer,"add  hl,hl");                      break;
		case 0x2a: sprintf (buffer,"ld   hl,($%04x)", ARGW(pc)); pc+=2;break;
		case 0x2b: sprintf (buffer,"dec  hl");                         break;
		case 0x2c: sprintf (buffer,"inc  l");                          break;
		case 0x2d: sprintf (buffer,"dec  l");                          break;
		case 0x2e: sprintf (buffer,"ld   l,$%02x", ARG(pc)); pc++;     break;
		case 0x2f: sprintf (buffer,"cpl");                             break;
		case 0x30: sprintf (buffer,"sim");                             break;
		case 0x31: sprintf (buffer,"ld   sp,$%04x", ARGW(pc)); pc+=2;  break;
		case 0x32: sprintf (buffer,"ld   ($%04x),a", ARGW(pc)); pc+=2; break;
		case 0x33: sprintf (buffer,"inc  sp");                         break;
		case 0x34: sprintf (buffer,"inc  (hl)");                       break;
		case 0x35: sprintf (buffer,"dec  (hl)");                       break;
		case 0x36: sprintf (buffer,"ld   (hl),$%02x", ARG(pc)); pc++;  break;
		case 0x37: sprintf (buffer,"scf");                             break;
		case 0x38: sprintf (buffer,"ld   de,sp+$%02x (*)",ARG(pc));pc++;break;
		case 0x39: sprintf (buffer,"add  hl,sp");                      break;
		case 0x3a: sprintf (buffer,"ld   a,($%04x)", ARGW(pc)); pc+=2; break;
		case 0x3b: sprintf (buffer,"dec  sp");                         break;
		case 0x3c: sprintf (buffer,"inc  a");                          break;
		case 0x3d: sprintf (buffer,"dec  a");                          break;
		case 0x3e: sprintf (buffer,"ld   a,$%02x", ARG(pc)); pc++;     break;
		case 0x3f: sprintf (buffer,"ccf");                             break;
		case 0x40: sprintf (buffer,"ld   b,b");                        break;
		case 0x41: sprintf (buffer,"ld   b,c");                        break;
		case 0x42: sprintf (buffer,"ld   b,d");                        break;
		case 0x43: sprintf (buffer,"ld   b,e");                        break;
		case 0x44: sprintf (buffer,"ld   b,h");                        break;
		case 0x45: sprintf (buffer,"ld   b,l");                        break;
		case 0x46: sprintf (buffer,"ld   b,(hl)");                     break;
		case 0x47: sprintf (buffer,"ld   b,a");                        break;
		case 0x48: sprintf (buffer,"ld   c,b");                        break;
		case 0x49: sprintf (buffer,"ld   c,c");                        break;
		case 0x4a: sprintf (buffer,"ld   c,d");                        break;
		case 0x4b: sprintf (buffer,"ld   c,e");                        break;
		case 0x4c: sprintf (buffer,"ld   c,h");                        break;
		case 0x4d: sprintf (buffer,"ld   c,l");                        break;
		case 0x4e: sprintf (buffer,"ld   c,(hl)");                     break;
		case 0x4f: sprintf (buffer,"ld   c,a");                        break;
		case 0x50: sprintf (buffer,"ld   d,b");                        break;
		case 0x51: sprintf (buffer,"ld   d,c");                        break;
		case 0x52: sprintf (buffer,"ld   d,d");                        break;
		case 0x53: sprintf (buffer,"ld   d,e");                        break;
		case 0x54: sprintf (buffer,"ld   d,h");                        break;
		case 0x55: sprintf (buffer,"ld   d,l");                        break;
		case 0x56: sprintf (buffer,"ld   d,(hl)");                     break;
		case 0x57: sprintf (buffer,"ld   d,a");                        break;
		case 0x58: sprintf (buffer,"ld   e,b");                        break;
		case 0x59: sprintf (buffer,"ld   e,c");                        break;
		case 0x5a: sprintf (buffer,"ld   e,d");                        break;
		case 0x5b: sprintf (buffer,"ld   e,e");                        break;
		case 0x5c: sprintf (buffer,"ld   e,h");                        break;
		case 0x5d: sprintf (buffer,"ld   e,l");                        break;
		case 0x5e: sprintf (buffer,"ld   e,(hl)");                     break;
		case 0x5f: sprintf (buffer,"ld   e,a");                        break;
		case 0x60: sprintf (buffer,"ld   h,b");                        break;
		case 0x61: sprintf (buffer,"ld   h,c");                        break;
		case 0x62: sprintf (buffer,"ld   h,d");                        break;
		case 0x63: sprintf (buffer,"ld   h,e");                        break;
		case 0x64: sprintf (buffer,"ld   h,h");                        break;
		case 0x65: sprintf (buffer,"ld   h,l");                        break;
		case 0x66: sprintf (buffer,"ld   h,(hl)");                     break;
		case 0x67: sprintf (buffer,"ld   h,a");                        break;
		case 0x68: sprintf (buffer,"ld   l,b");                        break;
		case 0x69: sprintf (buffer,"ld   l,c");                        break;
		case 0x6a: sprintf (buffer,"ld   l,d");                        break;
		case 0x6b: sprintf (buffer,"ld   l,e");                        break;
		case 0x6c: sprintf (buffer,"ld   l,h");                        break;
		case 0x6d: sprintf (buffer,"ld   l,l");                        break;
		case 0x6e: sprintf (buffer,"ld   l,(hl)");                     break;
		case 0x6f: sprintf (buffer,"ld   l,a");                        break;
		case 0x70: sprintf (buffer,"ld   (hl),b");                     break;
		case 0x71: sprintf (buffer,"ld   (hl),c");                     break;
		case 0x72: sprintf (buffer,"ld   (hl),d");                     break;
		case 0x73: sprintf (buffer,"ld   (hl),e");                     break;
		case 0x74: sprintf (buffer,"ld   (hl),h");                     break;
		case 0x75: sprintf (buffer,"ld   (hl),l");                     break;
		case 0x76: sprintf (buffer,"halt");                            break;
		case 0x77: sprintf (buffer,"ld   (hl),a");                     break;
		case 0x78: sprintf (buffer,"ld   a,b");                        break;
		case 0x79: sprintf (buffer,"ld   a,c");                        break;
		case 0x7a: sprintf (buffer,"ld   a,d");                        break;
		case 0x7b: sprintf (buffer,"ld   a,e");                        break;
		case 0x7c: sprintf (buffer,"ld   a,h");                        break;
		case 0x7d: sprintf (buffer,"ld   a,l");                        break;
		case 0x7e: sprintf (buffer,"ld   a,(hl)");                     break;
		case 0x7f: sprintf (buffer,"ld   a,a");                        break;
		case 0x80: sprintf (buffer,"add  a,b");                        break;
		case 0x81: sprintf (buffer,"add  a,c");                        break;
		case 0x82: sprintf (buffer,"add  a,d");                        break;
		case 0x83: sprintf (buffer,"add  a,e");                        break;
		case 0x84: sprintf (buffer,"add  a,h");                        break;
		case 0x85: sprintf (buffer,"add  a,l");                        break;
		case 0x86: sprintf (buffer,"add  a,(hl)");                     break;
		case 0x87: sprintf (buffer,"add  a,a");                        break;
		case 0x88: sprintf (buffer,"adc  a,b");                        break;
		case 0x89: sprintf (buffer,"adc  a,c");                        break;
		case 0x8a: sprintf (buffer,"adc  a,d");                        break;
		case 0x8b: sprintf (buffer,"adc  a,e");                        break;
		case 0x8c: sprintf (buffer,"adc  a,h");                        break;
		case 0x8d: sprintf (buffer,"adc  a,l");                        break;
		case 0x8e: sprintf (buffer,"adc  a,(hl)");                     break;
		case 0x8f: sprintf (buffer,"adc  a,a");                        break;
		case 0x90: sprintf (buffer,"sub  b");                          break;
		case 0x91: sprintf (buffer,"sub  c");                          break;
		case 0x92: sprintf (buffer,"sub  d");                          break;
		case 0x93: sprintf (buffer,"sub  e");                          break;
		case 0x94: sprintf (buffer,"sub  h");                          break;
		case 0x95: sprintf (buffer,"sub  l");                          break;
		case 0x96: sprintf (buffer,"sub  (hl)");                       break;
		case 0x97: sprintf (buffer,"sub  a");                          break;
		case 0x98: sprintf (buffer,"sbc  a,b");                        break;
		case 0x99: sprintf (buffer,"sbc  a,c");                        break;
		case 0x9a: sprintf (buffer,"sbc  a,d");                        break;
		case 0x9b: sprintf (buffer,"sbc  a,e");                        break;
		case 0x9c: sprintf (buffer,"sbc  a,h");                        break;
		case 0x9d: sprintf (buffer,"sbc  a,l");                        break;
		case 0x9e: sprintf (buffer,"sbc  a,(hl)");                     break;
		case 0x9f: sprintf (buffer,"sbc  a,a");                        break;
		case 0xa0: sprintf (buffer,"and  b");                          break;
		case 0xa1: sprintf (buffer,"and  c");                          break;
		case 0xa2: sprintf (buffer,"and  d");                          break;
		case 0xa3: sprintf (buffer,"and  e");                          break;
		case 0xa4: sprintf (buffer,"and  h");                          break;
		case 0xa5: sprintf (buffer,"and  l");                          break;
		case 0xa6: sprintf (buffer,"and  (hl)");                       break;
		case 0xa7: sprintf (buffer,"and  a");                          break;
		case 0xa8: sprintf (buffer,"xor  b");                          break;
		case 0xa9: sprintf (buffer,"xor  c");                          break;
		case 0xaa: sprintf (buffer,"xor  d");                          break;
		case 0xab: sprintf (buffer,"xor  e");                          break;
		case 0xac: sprintf (buffer,"xor  h");                          break;
		case 0xad: sprintf (buffer,"xor  l");                          break;
		case 0xae: sprintf (buffer,"xor  (hl)");                       break;
		case 0xaf: sprintf (buffer,"xor  a");                          break;
		case 0xb0: sprintf (buffer,"or   b");                          break;
		case 0xb1: sprintf (buffer,"or   c");                          break;
		case 0xb2: sprintf (buffer,"or   d");                          break;
		case 0xb3: sprintf (buffer,"or   e");                          break;
		case 0xb4: sprintf (buffer,"or   h");                          break;
		case 0xb5: sprintf (buffer,"or   l");                          break;
		case 0xb6: sprintf (buffer,"or   (hl)");                       break;
		case 0xb7: sprintf (buffer,"or   a");                          break;
		case 0xb8: sprintf (buffer,"cp   b");                          break;
		case 0xb9: sprintf (buffer,"cp   c");                          break;
		case 0xba: sprintf (buffer,"cp   d");                          break;
		case 0xbb: sprintf (buffer,"cp   e");                          break;
		case 0xbc: sprintf (buffer,"cp   h");                          break;
		case 0xbd: sprintf (buffer,"cp   l");                          break;
		case 0xbe: sprintf (buffer,"cp   (hl)");                       break;
		case 0xbf: sprintf (buffer,"cp   a");                          break;
		case 0xc0: sprintf (buffer,"ret  nz"); flags = DASMFLAG_STEP_OUT; break;
		case 0xc1: sprintf (buffer,"pop  bc");                         break;
		case 0xc2: sprintf (buffer,"jp   nz,$%04x", ARGW(pc)); pc+=2;  break;
		case 0xc3: sprintf (buffer,"jp   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xc4: sprintf (buffer,"call nz,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xc5: sprintf (buffer,"push bc");                         break;
		case 0xc6: sprintf (buffer,"add  a,$%02x", ARG(pc)); pc++;     break;
		case 0xc7: sprintf (buffer,"rst  $00"); flags = DASMFLAG_STEP_OVER; break;
		case 0xc8: sprintf (buffer,"ret  z"); flags = DASMFLAG_STEP_OUT; break;
		case 0xc9: sprintf (buffer,"ret"); flags = DASMFLAG_STEP_OUT;  break;
		case 0xca: sprintf (buffer,"jp   z,$%04x", ARGW(pc)); pc+=2;   break;
		case 0xcb: sprintf (buffer,"rst  v,$40 (*)"); flags = DASMFLAG_STEP_OVER; break;
		case 0xcc: sprintf (buffer,"call z,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xcd: sprintf (buffer,"call $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xce: sprintf (buffer,"adc  a,$%02x", ARG(pc)); pc++;     break;
		case 0xcf: sprintf (buffer,"rst  $08"); flags = DASMFLAG_STEP_OVER; break;
		case 0xd0: sprintf (buffer,"ret  nc"); flags = DASMFLAG_STEP_OUT; break;
		case 0xd1: sprintf (buffer,"pop  de");                         break;
		case 0xd2: sprintf (buffer,"jp   nc,$%04x", ARGW(pc)); pc+=2;  break;
		case 0xd3: sprintf (buffer,"out  ($%02x),a", ARG(pc)); pc++;   break;
		case 0xd4: sprintf (buffer,"call nc,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xd5: sprintf (buffer,"push de");                         break;
		case 0xd6: sprintf (buffer,"sub  $%02x", ARG(pc)); pc++;       break;
		case 0xd7: sprintf (buffer,"rst  $10"); flags = DASMFLAG_STEP_OVER; break;
		case 0xd8: sprintf (buffer,"ret  c");                          break;
		case 0xd9: sprintf (buffer,"ld   (de),hl (*)");                break;
		case 0xda: sprintf (buffer,"jp   c,$%04x", ARGW(pc)); pc+=2;   break;
		case 0xdb: sprintf (buffer,"in   a,($%02x)", ARG(pc)); pc++;   break;
		case 0xdc: sprintf (buffer,"call c,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xdd: sprintf (buffer,"jp   nx,$%04x (*)",ARGW(pc));pc+=2;break;
		case 0xde: sprintf (buffer,"sub  $%02x", ARG(pc)); pc++;       break;
		case 0xdf: sprintf (buffer,"rst  $18"); flags = DASMFLAG_STEP_OVER; break;
		case 0xe0: sprintf (buffer,"ret  pe");                         break;
		case 0xe1: sprintf (buffer,"pop  hl");                         break;
		case 0xe2: sprintf (buffer,"jp   pe,$%04x", ARGW(pc)); pc+=2;  break;
		case 0xe3: sprintf (buffer,"ex   (sp),hl");                    break;
		case 0xe4: sprintf (buffer,"call pe,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xe5: sprintf (buffer,"push hl");                         break;
		case 0xe6: sprintf (buffer,"and  $%02x", ARG(pc)); pc++;       break;
		case 0xe7: sprintf (buffer,"rst  $20"); flags = DASMFLAG_STEP_OVER; break;
		case 0xe8: sprintf (buffer,"ret  po");                         break;
		case 0xe9: sprintf (buffer,"jp   (hl)");                       break;
		case 0xea: sprintf (buffer,"jp   po,$%04x", ARGW(pc)); pc+=2;  break;
		case 0xeb: sprintf (buffer,"ex   de,hl");                      break;
		case 0xec: sprintf (buffer,"call po,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xed: sprintf (buffer,"ld   hl,(de) (*)");                break;
		case 0xee: sprintf (buffer,"xor  $%02x", ARG(pc)); pc++;       break;
		case 0xef: sprintf (buffer,"rst  $28"); flags = DASMFLAG_STEP_OVER; break;
		case 0xf0: sprintf (buffer,"ret  p");                          break;
		case 0xf1: sprintf (buffer,"pop  af");                         break;
		case 0xf2: sprintf (buffer,"jp   p,$%04x", ARGW(pc)); pc+=2;   break;
		case 0xf3: sprintf (buffer,"di");                              break;
		case 0xf4: sprintf (buffer,"cp   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xf5: sprintf (buffer,"push af");                         break;
		case 0xf6: sprintf (buffer,"or   $%02x", ARG(pc)); pc++;       break;
		case 0xf7: sprintf (buffer,"rst  $30"); flags = DASMFLAG_STEP_OVER; break;
		case 0xf8: sprintf (buffer,"ret  m");                          break;
		case 0xf9: sprintf (buffer,"ld   sp,hl");                      break;
		case 0xfa: sprintf (buffer,"jp   m,$%04x", ARGW(pc)); pc+=2;   break;
		case 0xfb: sprintf (buffer,"ei");                              break;
		case 0xfc: sprintf (buffer,"call m,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xfd: sprintf (buffer,"jp   x,$%04x (*)",ARGW(pc));pc+=2; break;
		case 0xfe: sprintf (buffer,"cp   $%02x", ARG(pc)); pc++;       break;
		case 0xff: sprintf (buffer,"rst  $38"); flags = DASMFLAG_STEP_OVER; break;
#else
		case 0x00: sprintf (buffer,"nop");                             break;
		case 0x01: sprintf (buffer,"lxi  b,$%04x", ARGW(pc)); pc+=2;   break;
		case 0x02: sprintf (buffer,"stax b");                          break;
		case 0x03: sprintf (buffer,"inx  b");                          break;
		case 0x04: sprintf (buffer,"inr  b");                          break;
		case 0x05: sprintf (buffer,"dcr  b");                          break;
		case 0x06: sprintf (buffer,"mvi  b,$%02x", ARG(pc)); pc++;     break;
		case 0x07: sprintf (buffer,"rlc");                             break;
		case 0x08: sprintf (buffer,"dsub (*)");                        break;
		case 0x09: sprintf (buffer,"dad  b");                          break;
		case 0x0a: sprintf (buffer,"ldax b");                          break;
		case 0x0b: sprintf (buffer,"dcx  b");                          break;
		case 0x0c: sprintf (buffer,"inr  c");                          break;
		case 0x0d: sprintf (buffer,"dcr  c");                          break;
		case 0x0e: sprintf (buffer,"mvi  c,$%02x", ARG(pc)); pc++;     break;
		case 0x0f: sprintf (buffer,"rrc");                             break;
		case 0x10: sprintf (buffer,"asrh (*)");                        break;
		case 0x11: sprintf (buffer,"lxi  d,$%04x", ARGW(pc)); pc+=2;   break;
		case 0x12: sprintf (buffer,"stax d");                          break;
		case 0x13: sprintf (buffer,"inx  d");                          break;
		case 0x14: sprintf (buffer,"inr  d");                          break;
		case 0x15: sprintf (buffer,"dcr  d");                          break;
		case 0x16: sprintf (buffer,"mvi  d,$%02x", ARG(pc)); pc++;     break;
		case 0x17: sprintf (buffer,"ral");                             break;
		case 0x18: sprintf (buffer,"rlde (*)");                        break;
		case 0x19: sprintf (buffer,"dad  d");                          break;
		case 0x1a: sprintf (buffer,"ldax d");                          break;
		case 0x1b: sprintf (buffer,"dcx  d");                          break;
		case 0x1c: sprintf (buffer,"inr  e");                          break;
		case 0x1d: sprintf (buffer,"dcr  e");                          break;
		case 0x1e: sprintf (buffer,"mvi  e,$%02x", ARG(pc)); pc++;     break;
		case 0x1f: sprintf (buffer,"rar");                             break;
		case 0x20: sprintf (buffer,"rim");                             break;
		case 0x21: sprintf (buffer,"lxi  h,$%04x", ARGW(pc)); pc+=2;   break;
		case 0x22: sprintf (buffer,"shld $%04x", ARGW(pc)); pc+=2;     break;
		case 0x23: sprintf (buffer,"inx  h");                          break;
		case 0x24: sprintf (buffer,"inr  h");                          break;
		case 0x25: sprintf (buffer,"dcr  h");                          break;
		case 0x26: sprintf (buffer,"mvi  h,$%02x", ARG(pc)); pc++;     break;
		case 0x27: sprintf (buffer,"daa");                             break;
		case 0x28: sprintf (buffer,"ldeh $%02x (*)", ARG(pc)); pc++;   break;
		case 0x29: sprintf (buffer,"dad  h");                          break;
		case 0x2a: sprintf (buffer,"lhld $%04x", ARGW(pc)); pc+=2;     break;
		case 0x2b: sprintf (buffer,"dcx  h");                          break;
		case 0x2c: sprintf (buffer,"inr  l");                          break;
		case 0x2d: sprintf (buffer,"dcr  l");                          break;
		case 0x2e: sprintf (buffer,"mvi  l,$%02x", ARG(pc)); pc++;     break;
		case 0x2f: sprintf (buffer,"cma");                             break;
		case 0x30: sprintf (buffer,"sim");                             break;
		case 0x31: sprintf (buffer,"lxi  sp,$%04x", ARGW(pc)); pc+=2;  break;
		case 0x32: sprintf (buffer,"stax $%04x", ARGW(pc)); pc+=2;     break;
		case 0x33: sprintf (buffer,"inx  sp");                         break;
		case 0x34: sprintf (buffer,"inr  m");                          break;
		case 0x35: sprintf (buffer,"dcr  m");                          break;
		case 0x36: sprintf (buffer,"mvi  m,$%02x", ARG(pc)); pc++;     break;
		case 0x37: sprintf (buffer,"stc");                             break;
		case 0x38: sprintf (buffer,"ldes $%02x", ARG(pc)); pc++;       break;
		case 0x39: sprintf (buffer,"dad sp");                          break;
		case 0x3a: sprintf (buffer,"ldax $%04x", ARGW(pc)); pc+=2;     break;
		case 0x3b: sprintf (buffer,"dcx  sp");                         break;
		case 0x3c: sprintf (buffer,"inr  a");                          break;
		case 0x3d: sprintf (buffer,"dcr  a");                          break;
		case 0x3e: sprintf (buffer,"mvi  a,$%02x", ARG(pc)); pc++;     break;
		case 0x3f: sprintf (buffer,"cmf");                             break;
		case 0x40: sprintf (buffer,"mov  b,b");                        break;
		case 0x41: sprintf (buffer,"mov  b,c");                        break;
		case 0x42: sprintf (buffer,"mov  b,d");                        break;
		case 0x43: sprintf (buffer,"mov  b,e");                        break;
		case 0x44: sprintf (buffer,"mov  b,h");                        break;
		case 0x45: sprintf (buffer,"mov  b,l");                        break;
		case 0x46: sprintf (buffer,"mov  b,m");                        break;
		case 0x47: sprintf (buffer,"mov  b,a");                        break;
		case 0x48: sprintf (buffer,"mov  c,b");                        break;
		case 0x49: sprintf (buffer,"mov  c,c");                        break;
		case 0x4a: sprintf (buffer,"mov  c,d");                        break;
		case 0x4b: sprintf (buffer,"mov  c,e");                        break;
		case 0x4c: sprintf (buffer,"mov  c,h");                        break;
		case 0x4d: sprintf (buffer,"mov  c,l");                        break;
		case 0x4e: sprintf (buffer,"mov  c,m");                        break;
		case 0x4f: sprintf (buffer,"mov  c,a");                        break;
		case 0x50: sprintf (buffer,"mov  d,b");                        break;
		case 0x51: sprintf (buffer,"mov  d,c");                        break;
		case 0x52: sprintf (buffer,"mov  d,d");                        break;
		case 0x53: sprintf (buffer,"mov  d,e");                        break;
		case 0x54: sprintf (buffer,"mov  d,h");                        break;
		case 0x55: sprintf (buffer,"mov  d,l");                        break;
		case 0x56: sprintf (buffer,"mov  d,m");                        break;
		case 0x57: sprintf (buffer,"mov  d,a");                        break;
		case 0x58: sprintf (buffer,"mov  e,b");                        break;
		case 0x59: sprintf (buffer,"mov  e,c");                        break;
		case 0x5a: sprintf (buffer,"mov  e,d");                        break;
		case 0x5b: sprintf (buffer,"mov  e,e");                        break;
		case 0x5c: sprintf (buffer,"mov  e,h");                        break;
		case 0x5d: sprintf (buffer,"mov  e,l");                        break;
		case 0x5e: sprintf (buffer,"mov  e,m");                        break;
		case 0x5f: sprintf (buffer,"mov  e,a");                        break;
		case 0x60: sprintf (buffer,"mov  h,b");                        break;
		case 0x61: sprintf (buffer,"mov  h,c");                        break;
		case 0x62: sprintf (buffer,"mov  h,d");                        break;
		case 0x63: sprintf (buffer,"mov  h,e");                        break;
		case 0x64: sprintf (buffer,"mov  h,h");                        break;
		case 0x65: sprintf (buffer,"mov  h,l");                        break;
		case 0x66: sprintf (buffer,"mov  h,m");                        break;
		case 0x67: sprintf (buffer,"mov  h,a");                        break;
		case 0x68: sprintf (buffer,"mov  l,b");                        break;
		case 0x69: sprintf (buffer,"mov  l,c");                        break;
		case 0x6a: sprintf (buffer,"mov  l,d");                        break;
		case 0x6b: sprintf (buffer,"mov  l,e");                        break;
		case 0x6c: sprintf (buffer,"mov  l,h");                        break;
		case 0x6d: sprintf (buffer,"mov  l,l");                        break;
		case 0x6e: sprintf (buffer,"mov  l,m");                        break;
		case 0x6f: sprintf (buffer,"mov  l,a");                        break;
		case 0x70: sprintf (buffer,"mov  m,b");                        break;
		case 0x71: sprintf (buffer,"mov  m,c");                        break;
		case 0x72: sprintf (buffer,"mov  m,d");                        break;
		case 0x73: sprintf (buffer,"mov  m,e");                        break;
		case 0x74: sprintf (buffer,"mov  m,h");                        break;
		case 0x75: sprintf (buffer,"mov  m,l");                        break;
		case 0x76: sprintf (buffer,"hlt");                             break;
		case 0x77: sprintf (buffer,"mov  m,a");                        break;
		case 0x78: sprintf (buffer,"mov  a,b");                        break;
		case 0x79: sprintf (buffer,"mov  a,c");                        break;
		case 0x7a: sprintf (buffer,"mov  a,d");                        break;
		case 0x7b: sprintf (buffer,"mov  a,e");                        break;
		case 0x7c: sprintf (buffer,"mov  a,h");                        break;
		case 0x7d: sprintf (buffer,"mov  a,l");                        break;
		case 0x7e: sprintf (buffer,"mov  a,m");                        break;
		case 0x7f: sprintf (buffer,"mov  a,a");                        break;
		case 0x80: sprintf (buffer,"add  b");                          break;
		case 0x81: sprintf (buffer,"add  c");                          break;
		case 0x82: sprintf (buffer,"add  d");                          break;
		case 0x83: sprintf (buffer,"add  e");                          break;
		case 0x84: sprintf (buffer,"add  h");                          break;
		case 0x85: sprintf (buffer,"add  l");                          break;
		case 0x86: sprintf (buffer,"add  m");                          break;
		case 0x87: sprintf (buffer,"add  a");                          break;
		case 0x88: sprintf (buffer,"adc  b");                          break;
		case 0x89: sprintf (buffer,"adc  c");                          break;
		case 0x8a: sprintf (buffer,"adc  d");                          break;
		case 0x8b: sprintf (buffer,"adc  e");                          break;
		case 0x8c: sprintf (buffer,"adc  h");                          break;
		case 0x8d: sprintf (buffer,"adc  l");                          break;
		case 0x8e: sprintf (buffer,"adc  m");                          break;
		case 0x8f: sprintf (buffer,"adc  a");                          break;
		case 0x90: sprintf (buffer,"sub  b");                          break;
		case 0x91: sprintf (buffer,"sub  c");                          break;
		case 0x92: sprintf (buffer,"sub  d");                          break;
		case 0x93: sprintf (buffer,"sub  e");                          break;
		case 0x94: sprintf (buffer,"sub  h");                          break;
		case 0x95: sprintf (buffer,"sub  l");                          break;
		case 0x96: sprintf (buffer,"sub  m");                          break;
		case 0x97: sprintf (buffer,"sub  a");                          break;
		case 0x98: sprintf (buffer,"sbb  b");                          break;
		case 0x99: sprintf (buffer,"sbb  c");                          break;
		case 0x9a: sprintf (buffer,"sbb  d");                          break;
		case 0x9b: sprintf (buffer,"sbb  e");                          break;
		case 0x9c: sprintf (buffer,"sbb  h");                          break;
		case 0x9d: sprintf (buffer,"sbb  l");                          break;
		case 0x9e: sprintf (buffer,"sbb  m");                          break;
		case 0x9f: sprintf (buffer,"sbb  a");                          break;
		case 0xa0: sprintf (buffer,"ana  b");                          break;
		case 0xa1: sprintf (buffer,"ana  c");                          break;
		case 0xa2: sprintf (buffer,"ana  d");                          break;
		case 0xa3: sprintf (buffer,"ana  e");                          break;
		case 0xa4: sprintf (buffer,"ana  h");                          break;
		case 0xa5: sprintf (buffer,"ana  l");                          break;
		case 0xa6: sprintf (buffer,"ana  m");                          break;
		case 0xa7: sprintf (buffer,"ana  a");                          break;
		case 0xa8: sprintf (buffer,"xra  b");                          break;
		case 0xa9: sprintf (buffer,"xra  c");                          break;
		case 0xaa: sprintf (buffer,"xra  d");                          break;
		case 0xab: sprintf (buffer,"xra  e");                          break;
		case 0xac: sprintf (buffer,"xra  h");                          break;
		case 0xad: sprintf (buffer,"xra  l");                          break;
		case 0xae: sprintf (buffer,"xra  m");                          break;
		case 0xaf: sprintf (buffer,"xra  a");                          break;
		case 0xb0: sprintf (buffer,"ora  b");                          break;
		case 0xb1: sprintf (buffer,"ora  c");                          break;
		case 0xb2: sprintf (buffer,"ora  d");                          break;
		case 0xb3: sprintf (buffer,"ora  e");                          break;
		case 0xb4: sprintf (buffer,"ora  h");                          break;
		case 0xb5: sprintf (buffer,"ora  l");                          break;
		case 0xb6: sprintf (buffer,"ora  m");                          break;
		case 0xb7: sprintf (buffer,"ora  a");                          break;
		case 0xb8: sprintf (buffer,"cmp  b");                          break;
		case 0xb9: sprintf (buffer,"cmp  c");                          break;
		case 0xba: sprintf (buffer,"cmp  d");                          break;
		case 0xbb: sprintf (buffer,"cmp  e");                          break;
		case 0xbc: sprintf (buffer,"cmp  h");                          break;
		case 0xbd: sprintf (buffer,"cmp  l");                          break;
		case 0xbe: sprintf (buffer,"cmp  m");                          break;
		case 0xbf: sprintf (buffer,"cmp  a");                          break;
		case 0xc0: sprintf (buffer,"rnz"); flags = DASMFLAG_STEP_OUT;  break;
		case 0xc1: sprintf (buffer,"pop  b");                          break;
		case 0xc2: sprintf (buffer,"jnz  $%04x", ARGW(pc)); pc+=2;     break;
		case 0xc3: sprintf (buffer,"jmp  $%04x", ARGW(pc)); pc+=2;     break;
		case 0xc4: sprintf (buffer,"cnz  $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xc5: sprintf (buffer,"push b");                          break;
		case 0xc6: sprintf (buffer,"adi  $%02x", ARG(pc)); pc++;       break;
		case 0xc7: sprintf (buffer,"rst  0"); flags = DASMFLAG_STEP_OVER; break;
		case 0xc8: sprintf (buffer,"rz"); flags = DASMFLAG_STEP_OUT;   break;
		case 0xc9: sprintf (buffer,"ret"); flags = DASMFLAG_STEP_OUT;  break;
		case 0xca: sprintf (buffer,"jz   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xcb: sprintf (buffer,"rstv 8 (*)"); flags = DASMFLAG_STEP_OVER; break;
		case 0xcc: sprintf (buffer,"cz   $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xcd: sprintf (buffer,"call $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xce: sprintf (buffer,"aci  $%02x", ARG(pc)); pc++;       break;
		case 0xcf: sprintf (buffer,"rst  1"); flags = DASMFLAG_STEP_OVER; break;
		case 0xd0: sprintf (buffer,"rnc"); flags = DASMFLAG_STEP_OUT;  break;
		case 0xd1: sprintf (buffer,"pop  d");                          break;
		case 0xd2: sprintf (buffer,"jnc  $%04x", ARGW(pc)); pc+=2;     break;
		case 0xd3: sprintf (buffer,"out  $%02x", ARG(pc)); pc++;       break;
		case 0xd4: sprintf (buffer,"cnc  $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xd5: sprintf (buffer,"push d");                          break;
		case 0xd6: sprintf (buffer,"sui  $%02x", ARG(pc)); pc++;       break;
		case 0xd7: sprintf (buffer,"rst  2"); flags = DASMFLAG_STEP_OVER; break;
		case 0xd8: sprintf (buffer,"rc"); flags = DASMFLAG_STEP_OUT;   break;
		case 0xd9: sprintf (buffer,"shlx d (*)");                      break;
		case 0xda: sprintf (buffer,"jc   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xdb: sprintf (buffer,"in   $%02x", ARG(pc)); pc++;       break;
		case 0xdc: sprintf (buffer,"cc   $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xdd: sprintf (buffer,"jnx  $%04x (*)", ARGW(pc)); pc+=2; break;
		case 0xde: sprintf (buffer,"sbi  $%02x", ARG(pc)); pc++;       break;
		case 0xdf: sprintf (buffer,"rst  3"); flags = DASMFLAG_STEP_OVER; break;
		case 0xe0: sprintf (buffer,"rpo"); flags = DASMFLAG_STEP_OUT;  break;
		case 0xe1: sprintf (buffer,"pop  h");                          break;
		case 0xe2: sprintf (buffer,"jpo  $%04x", ARGW(pc)); pc+=2;     break;
		case 0xe3: sprintf (buffer,"xthl");                            break;
		case 0xe4: sprintf (buffer,"cpo  $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xe5: sprintf (buffer,"push h");                          break;
		case 0xe6: sprintf (buffer,"ani  $%02x", ARG(pc)); pc++;       break;
		case 0xe7: sprintf (buffer,"rst  4"); flags = DASMFLAG_STEP_OVER; break;
		case 0xe8: sprintf (buffer,"rpe"); flags = DASMFLAG_STEP_OUT;  break;
		case 0xe9: sprintf (buffer,"pchl");                            break;
		case 0xea: sprintf (buffer,"jpe  $%04x", ARGW(pc)); pc+=2;     break;
		case 0xeb: sprintf (buffer,"xchg");                            break;
		case 0xec: sprintf (buffer,"cpe  $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xed: sprintf (buffer,"lhlx d (*)");                      break;
		case 0xee: sprintf (buffer,"xri  $%02x", ARG(pc)); pc++;       break;
		case 0xef: sprintf (buffer,"rst  5"); flags = DASMFLAG_STEP_OVER; break;
		case 0xf0: sprintf (buffer,"rp"); flags = DASMFLAG_STEP_OUT;   break;
		case 0xf1: sprintf (buffer,"pop  a");                          break;
		case 0xf2: sprintf (buffer,"jp   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xf3: sprintf (buffer,"di");                              break;
		case 0xf4: sprintf (buffer,"cp   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xf5: sprintf (buffer,"push a");                          break;
		case 0xf6: sprintf (buffer,"ori  $%02x", ARG(pc)); pc++;       break;
		case 0xf7: sprintf (buffer,"rst  6"); flags = DASMFLAG_STEP_OVER; break;
		case 0xf8: sprintf (buffer,"rm"); flags = DASMFLAG_STEP_OUT;   break;
		case 0xf9: sprintf (buffer,"sphl");                            break;
		case 0xfa: sprintf (buffer,"jm   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xfb: sprintf (buffer,"ei");                              break;
		case 0xfc: sprintf (buffer,"cm   $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xfd: sprintf (buffer,"jx   $%04x (*)", ARGW(pc)); pc+=2; break;
		case 0xfe: sprintf (buffer,"cpi  $%02x", ARG(pc)); pc++;       break;
		case 0xff: sprintf (buffer,"rst  7"); flags = DASMFLAG_STEP_OVER; break;
#endif
	}
	return (pc - PC) | flags | DASMFLAG_SUPPORTED;
}

