/*****************************************************************************
 *
 *   minc4510.h
 *   Base macros for 4510 CPU files
 *
 *****************************************************************************/


/* 4510 flags */
#define F_C	0x01
#define F_Z	0x02
#define F_I	0x04
#define F_D	0x08
#define F_B	0x10
#define F_E	0x20
#define F_V	0x40
#define F_N	0x80

/* some shortcuts for improved readability */
#define A	cpustate->a
#define X	cpustate->x
#define Y	cpustate->y
#define P	cpustate->p
#define Z	cpustate->z
#define B	cpustate->zp.b.h
#define SW	cpustate->sp.w.l
#define SPL	cpustate->sp.b.l
#define SPH	cpustate->sp.b.h
#define SPD	cpustate->sp.d

#define NZ	cpustate->nz

#define EAL	cpustate->ea.b.l
#define EAH	cpustate->ea.b.h
#define EAW	cpustate->ea.w.l
#define EAD	cpustate->ea.d

#define ZPL	cpustate->zp.b.l
#define ZPH	cpustate->zp.b.h
#define ZPW	cpustate->zp.w.l
#define ZPD	cpustate->zp.d

#define PCL	cpustate->pc.b.l
#define PCH	cpustate->pc.b.h
#define PCW	cpustate->pc.w.l
#define PCD	cpustate->pc.d

#define PPC	cpustate->ppc.d

#define IRQ_STATE	cpustate->irq_state
#define AFTER_CLI	cpustate->after_cli

#define M4510_MEM(addr)	(cpustate->mem[(addr)>>13]+(addr))

#define PEEK_OP()	memory_decrypted_read_byte(cpustate->space, M4510_MEM(PCW))

#define RDMEM(addr)			memory_read_byte_8le(cpustate->space, M4510_MEM(addr)); cpustate->icount -= 1
#define WRMEM(addr,data)	memory_write_byte_8le(cpustate->space, M4510_MEM(addr),data); cpustate->icount -= 1

/***************************************************************
 *  RDOP    read an opcode
 ***************************************************************/
#undef RDOP
#define RDOP() m4510_cpu_readop(cpustate); cpustate->icount -= 1

/***************************************************************
 *  RDOPARG read an opcode argument
 ***************************************************************/
#undef RDOPARG
#define RDOPARG() m4510_cpu_readop_arg(cpustate); cpustate->icount -= 1
