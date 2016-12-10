static struct {
	struct {
		WREGS w[256];
		BREGS b[256];
	} reg;
	struct {
		WREGS w[256];
		BREGS b[256];
	} RM;
} Mod_RM;

#define RegWord(ModRM) nec_state->regs.w[Mod_RM.reg.w[ModRM]]
#define RegByte(ModRM) nec_state->regs.b[Mod_RM.reg.b[ModRM]]

#define GetRMWord(ModRM) \
	((ModRM) >= 0xc0 ? nec_state->regs.w[Mod_RM.RM.w[ModRM]] : ( (*GetEA[ModRM])(nec_state), read_word( EA ) ))

#define PutbackRMWord(ModRM,val)			     \
{							     \
	if (ModRM >= 0xc0) nec_state->regs.w[Mod_RM.RM.w[ModRM]]=val; \
    else write_word(EA,val);  \
}

#define GetnextRMWord read_word((EA&0xf0000)|((EA+2)&0xffff))

#define PutRMWord(ModRM,val)				\
{							\
	if (ModRM >= 0xc0)				\
		nec_state->regs.w[Mod_RM.RM.w[ModRM]]=val;	\
	else {						\
		(*GetEA[ModRM])(nec_state);			\
		write_word( EA ,val);			\
	}						\
}

#define PutImmRMWord(ModRM) 				\
{							\
	WORD val;					\
	if (ModRM >= 0xc0)				\
		nec_state->regs.w[Mod_RM.RM.w[ModRM]] = FETCHWORD(); \
	else {						\
		(*GetEA[ModRM])(nec_state);			\
		val = FETCHWORD();				\
		write_word( EA , val);			\
	}						\
}

#define GetRMByte(ModRM) \
	((ModRM) >= 0xc0 ? nec_state->regs.b[Mod_RM.RM.b[ModRM]] : read_byte( (*GetEA[ModRM])(nec_state) ))

#define PutRMByte(ModRM,val)				\
{							\
	if (ModRM >= 0xc0)				\
		nec_state->regs.b[Mod_RM.RM.b[ModRM]]=val;	\
	else						\
		write_byte( (*GetEA[ModRM])(nec_state) ,val);	\
}

#define PutImmRMByte(ModRM) 				\
{							\
	if (ModRM >= 0xc0)				\
		nec_state->regs.b[Mod_RM.RM.b[ModRM]]=FETCH();	\
	else {						\
		(*GetEA[ModRM])(nec_state);			\
		write_byte( EA , FETCH() );		\
	}						\
}

#define PutbackRMByte(ModRM,val)			\
{							\
	if (ModRM >= 0xc0)				\
		nec_state->regs.b[Mod_RM.RM.b[ModRM]]=val;	\
	else						\
		write_byte(EA,val);			\
}

#define DEF_br8							\
	UINT32 ModRM = FETCH(),src,dst;		\
	src = RegByte(ModRM);				\
    dst = GetRMByte(ModRM)

#define DEF_wr16						\
	UINT32 ModRM = FETCH(),src,dst;		\
	src = RegWord(ModRM);				\
    dst = GetRMWord(ModRM)

#define DEF_r8b							\
	UINT32 ModRM = FETCH(),src,dst;		\
	dst = RegByte(ModRM);				\
    src = GetRMByte(ModRM)

#define DEF_r16w						\
	UINT32 ModRM = FETCH(),src,dst;		\
	dst = RegWord(ModRM);				\
    src = GetRMWord(ModRM)

#define DEF_ald8						\
	UINT32 src = FETCH();					\
	UINT32 dst = nec_state->regs.b[AL]

#define DEF_axd16						\
	UINT32 src = FETCH();				\
	UINT32 dst = nec_state->regs.w[AW];			\
    src += (FETCH() << 8)
