/* assumed
   res=left+right+c
   sbc is like adc with inverted carry and right side
   (decrement, compare the same)
   (like in the m6502 processors)
*/
INLINE UINT8 lh5801_add_generic(lh5801_state *cpustate, int left, int right, int carry)
{
	int res=left+right+carry;
	int v,c;

	cpustate->t&=~(H|V|Z|C);

	if (!(res&0xff)) cpustate->t|=Z;
	c=res&0x100;
	if (c) cpustate->t|=C;
	if (((left&0xf)+(right&0xf)+carry)&0x10) cpustate->t|=H;
	v=((left&0x7f)+(right&0x7f)+carry)&0x80;
	if ( (c&&!v)||(!c&&v) ) cpustate->t|=V;

	return res;
}

INLINE UINT16 lh5801_readop_word(lh5801_state *cpustate)
{
	UINT16 r;
	r=memory_decrypted_read_byte(cpustate->program,P++)<<8;
	r|=memory_decrypted_read_byte(cpustate->program,P++);
	return r;
}


INLINE void lh5801_adc(lh5801_state *cpustate, UINT8 data)
{
	cpustate->a=lh5801_add_generic(cpustate,cpustate->a,data,cpustate->t&C);
}

INLINE void lh5801_add_mem(lh5801_state *cpustate, int addr, UINT8 data)
{
	int v=lh5801_add_generic(cpustate,memory_read_byte(cpustate->program,addr),data,0);
	memory_write_byte(cpustate->program,addr,v);
}

INLINE void lh5801_adr(lh5801_state *cpustate, PAIR *reg)
{
	reg->b.l=lh5801_add_generic(cpustate,reg->b.l,cpustate->a,0);
	if (cpustate->t&C) {
		reg->b.h++;
	}
}

INLINE void lh5801_sbc(lh5801_state *cpustate, UINT8 data)
{
	cpustate->a=lh5801_add_generic(cpustate,cpustate->a,data^0xff,(cpustate->t&C)^1);
}

INLINE void lh5801_cpa(lh5801_state *cpustate, UINT8 a, UINT8 b)
{
	lh5801_add_generic(cpustate,a, b^0xff, 1);
}

INLINE UINT8 lh5801_decimaladd_generic(lh5801_state *cpustate, int left, int right, int carry)
{
	int res=(left&0xf)+(right&0xf)+carry;
	cpustate->t&=~(H|V|Z|C);

	if (res>=10) {
		res+=6;
		cpustate->t|=H;
	}
	res+=(left&0xf0)+(right&0xf0);
	if (res>=0xa0) {
		res+=0x60;
		cpustate->t|=C;
	}
	if (!(res&0xff)) cpustate->t|=Z;
	//v???
	return res;
}

INLINE void lh5801_dca(lh5801_state *cpustate, UINT8 data)
{
	cpustate->a=lh5801_decimaladd_generic(cpustate, cpustate->a, data, cpustate->t&C);
}

INLINE void lh5801_dcs(lh5801_state *cpustate, UINT8 data)
{
	cpustate->a=lh5801_decimaladd_generic(cpustate, cpustate->a, data^0xff, (cpustate->t&C)^1);
}

INLINE void lh5801_and(lh5801_state *cpustate, UINT8 data)
{
	cpustate->a&=data;
	cpustate->t&=~Z;
	if (!cpustate->a) cpustate->t|=Z;
}

INLINE void lh5801_and_mem(lh5801_state *cpustate, int addr, UINT8 data)
{
	data&=memory_read_byte(cpustate->program,addr);
	cpustate->t&=~Z;
	if (!data) cpustate->t|=Z;
	memory_write_byte(cpustate->program,addr,data);
}

INLINE void lh5801_bit(lh5801_state *cpustate, UINT8 a, UINT8 b)
{
	cpustate->t&=~Z;
	if (!(a&b)) cpustate->t|=Z;
}

INLINE void lh5801_eor(lh5801_state *cpustate, UINT8 data)
{
	cpustate->a^=data;
	cpustate->t&=~Z;
	if (!cpustate->a) cpustate->t|=Z;
}

INLINE void lh5801_ora(lh5801_state *cpustate, UINT8 data)
{
	cpustate->a^=data;
	cpustate->t&=~Z;
	if (!cpustate->a) cpustate->t|=Z;
}

INLINE void lh5801_ora_mem(lh5801_state *cpustate, int addr, UINT8 data)
{
	data|=memory_read_byte(cpustate->program,addr);
	cpustate->t&=~Z;
	if (!data) cpustate->t|=Z;
	memory_write_byte(cpustate->program,addr,data);
}

INLINE void lh5801_lda(lh5801_state *cpustate, UINT8 data)
{
	cpustate->a=data;
	cpustate->t&=~Z;
	if (!cpustate->a) cpustate->t|=Z;
}

INLINE void lh5801_lde(lh5801_state *cpustate, PAIR *reg)
{
	// or z flag depends on reg
	cpustate->a=memory_read_byte(cpustate->program,reg->w.l--);
	cpustate->t&=~Z;
	if (!cpustate->a) cpustate->t|=Z;
}

INLINE void lh5801_sde(lh5801_state *cpustate, PAIR *reg)
{
	memory_write_byte(cpustate->program,reg->w.l--, cpustate->a);
}

INLINE void lh5801_lin(lh5801_state *cpustate, PAIR *reg)
{
	// or z flag depends on reg
	cpustate->a=memory_read_byte(cpustate->program,reg->w.l++);
	cpustate->t&=~Z;
	if (!cpustate->a) cpustate->t|=Z;
}

INLINE void lh5801_sin(lh5801_state *cpustate, PAIR *reg)
{
	memory_write_byte(cpustate->program,reg->w.l++, cpustate->a);
}

INLINE void lh5801_dec(lh5801_state *cpustate, UINT8 *adr)
{
	*adr=lh5801_add_generic(cpustate,*adr,0xff,0);
}

INLINE void lh5801_inc(lh5801_state *cpustate, UINT8 *adr)
{
	*adr=lh5801_add_generic(cpustate,*adr,1,0);
}

INLINE void lh5801_pop(lh5801_state *cpustate)
{
	cpustate->a=memory_read_byte(cpustate->program,++S);
	cpustate->t&=~Z;
	if (!cpustate->a) cpustate->t|=Z;
}

INLINE void lh5801_pop_word(lh5801_state *cpustate, PAIR *reg)
{
	reg->b.h=memory_read_byte(cpustate->program,++S);
	reg->b.l=memory_read_byte(cpustate->program,++S);
	// z flag?
}

INLINE void lh5801_rtn(lh5801_state *cpustate)
{
	P=memory_read_byte(cpustate->program,++S)<<8;
	P|=memory_read_byte(cpustate->program,++S);
}

INLINE void lh5801_rti(lh5801_state *cpustate)
{
	P=memory_read_byte(cpustate->program,++S)<<8;
	P|=memory_read_byte(cpustate->program,++S);
	cpustate->t=memory_read_byte(cpustate->program,++S);
}

INLINE void lh5801_push(lh5801_state *cpustate, UINT8 data)
{
	memory_write_byte(cpustate->program,S--, data);
}

INLINE void lh5801_push_word(lh5801_state *cpustate, UINT16 data)
{
	memory_write_byte(cpustate->program,S--, data&0xff);
	memory_write_byte(cpustate->program,S--, data>>8);
}

INLINE void lh5801_jmp(lh5801_state *cpustate, UINT16 adr)
{
	P=adr;
}

INLINE void lh5801_branch_plus(lh5801_state *cpustate, int doit)
{
	UINT8 t=memory_decrypted_read_byte(cpustate->program,P++);
	if (doit) {
		cpustate->icount-=3;
		P+=t;
	}
}

INLINE void lh5801_branch_minus(lh5801_state *cpustate, int doit)
{
	UINT8 t=memory_decrypted_read_byte(cpustate->program,P++);
	if (doit) {
		cpustate->icount-=3;
		P-=t;
	}
}

INLINE void lh5801_lop(lh5801_state *cpustate)
{
	UINT8 t=memory_decrypted_read_byte(cpustate->program,P++);
	cpustate->icount-=8;
	if (UL--) {
		cpustate->icount-=3;
		P-=t;
	}
}

INLINE void lh5801_sjp(lh5801_state *cpustate)
{
	UINT16 n=lh5801_readop_word(cpustate);
	lh5801_push_word(cpustate,P);
	P=n;
}

INLINE void lh5801_vector(lh5801_state *cpustate, int doit, int nr)
{
	if (doit) {
		lh5801_push_word(cpustate,P);
		P=memory_read_byte(cpustate->program,0xff00+nr)<<8;
		P|=memory_read_byte(cpustate->program,0xff00+nr+1);
		cpustate->icount-=21-8;
	}
	cpustate->t&=~Z; // after the jump!?
}

INLINE void lh5801_aex(lh5801_state *cpustate)
{
	UINT8 t=cpustate->a;
	cpustate->a=(t<<4)|(t>>4);
	// flags?
}

INLINE void lh5801_drl(lh5801_state *cpustate, int adr)
{
	UINT16 t=cpustate->a|(memory_read_byte(cpustate->program,adr)<<8);

	cpustate->a=t>>8;
	memory_write_byte(cpustate->program,adr,t>>4);
}

INLINE void lh5801_drr(lh5801_state *cpustate, int adr)
{
	UINT16 t=memory_read_byte(cpustate->program,adr)|(cpustate->a<<8);

	cpustate->a=t;
	memory_write_byte(cpustate->program,adr,t>>4);
}

INLINE void lh5801_rol(lh5801_state *cpustate)
{
	// maybe use of the adder
	int n=(cpustate->a<<1)|(cpustate->t&C);
	cpustate->a=n;
	// flags cvhz
	cpustate->t&=~(C&Z);
	if (n&0x100) cpustate->t|=C;
	if (!(n&0xff)) cpustate->t|=Z;
}

INLINE void lh5801_ror(lh5801_state *cpustate)
{
	int n=cpustate->a|((cpustate->t&C)<<8);
	cpustate->a=n>>1;
	// flags cvhz
	cpustate->t&=~(C&Z);
	cpustate->t|=(n&C);
	if (!(n&0x1fe)) cpustate->t|=Z;
}

INLINE void lh5801_shl(lh5801_state *cpustate)
{
	int nc=cpustate->a&0x80;
	cpustate->a<<=1;
	// flags cvhz
	cpustate->t&=~(C&Z);
	if (nc) cpustate->t|=C;
	if (!cpustate->a) cpustate->t|=Z;
}

INLINE void lh5801_shr(lh5801_state *cpustate)
{
	int nc=cpustate->a&1;
	cpustate->a>>=1;
	// flags cvhz
	cpustate->t&=~(C|Z);
	if (nc) cpustate->t|=C;
	if (!cpustate->a) cpustate->t|=Z;
}

INLINE void lh5801_am(lh5801_state *cpustate, int value)
{
	cpustate->tm=value;
	// jfkas?jfkl?jkd?
}

INLINE void lh5801_ita(lh5801_state *cpustate)
{
	if (cpustate->config&&cpustate->config->in) {
		cpustate->a=cpustate->config->in(cpustate->device);
	} else {
		cpustate->a=0;
	}
	cpustate->t&=~Z;
	if (!cpustate->a) cpustate->t|=Z;
}

static void lh5801_instruction_fd(lh5801_state *cpustate)
{
	int oper;
	int adr;

	oper=memory_decrypted_read_byte(cpustate->program,P++);
	switch (oper) {
	case 0x01: lh5801_sbc(cpustate,memory_read_byte(cpustate->program,0x10000|X)); cpustate->icount-=11;break;
	case 0x03: lh5801_adc(cpustate,memory_read_byte(cpustate->program,0x10000|X)); cpustate->icount-=11;break;
	case 0x05: lh5801_lda(cpustate,memory_read_byte(cpustate->program,0x10000|X)); cpustate->icount-=10;break;
	case 0x07: lh5801_cpa(cpustate,cpustate->a, memory_read_byte(cpustate->program,0x10000|X)); cpustate->icount-=11;break;
	case 0x08: X=X;cpustate->icount-=11;break; //!!!
	case 0x09: lh5801_and(cpustate,memory_read_byte(cpustate->program,0x10000|X)); cpustate->icount-=11;break;
	case 0x0a: lh5801_pop_word(cpustate,&cpustate->x); cpustate->icount-=15;break;
	case 0x0b: lh5801_ora(cpustate,memory_read_byte(cpustate->program,0x10000|X)); cpustate->icount-=11;break;
	case 0x0c: lh5801_dcs(cpustate,memory_read_byte(cpustate->program,0x10000|X)); cpustate->icount-=17; break;
	case 0x0d: lh5801_eor(cpustate,memory_read_byte(cpustate->program,0x10000|X)); cpustate->icount-=11;break;
	case 0x0e: memory_write_byte(cpustate->program,0x10000|X,cpustate->a); cpustate->icount-=10;break;
	case 0x0f: lh5801_bit(cpustate,memory_read_byte(cpustate->program,0x10000|X),cpustate->a); cpustate->icount-=11;break;
	case 0x11: lh5801_sbc(cpustate,memory_read_byte(cpustate->program,0x10000|Y)); cpustate->icount-=11;break;
	case 0x13: lh5801_adc(cpustate,memory_read_byte(cpustate->program,0x10000|Y)); cpustate->icount-=11;break;
	case 0x15: lh5801_lda(cpustate,memory_read_byte(cpustate->program,0x10000|Y)); cpustate->icount-=10;break;
	case 0x17: lh5801_cpa(cpustate,cpustate->a, memory_read_byte(cpustate->program,0x10000|Y)); cpustate->icount-=11;break;
	case 0x18: X=Y;cpustate->icount-=11;break;
	case 0x19: lh5801_and(cpustate,memory_read_byte(cpustate->program,0x10000|Y)); cpustate->icount-=11;break;
	case 0x1a: lh5801_pop_word(cpustate,&cpustate->y); cpustate->icount-=15;break;
	case 0x1b: lh5801_ora(cpustate,memory_read_byte(cpustate->program,0x10000|Y)); cpustate->icount-=11;break;
	case 0x1c: lh5801_dcs(cpustate,memory_read_byte(cpustate->program,0x10000|Y)); cpustate->icount-=17; break;
	case 0x1d: lh5801_eor(cpustate,memory_read_byte(cpustate->program,0x10000|Y)); cpustate->icount-=11;break;
	case 0x1e: memory_write_byte(cpustate->program,0x10000|Y,cpustate->a); cpustate->icount-=10;break;
	case 0x1f: lh5801_bit(cpustate,memory_read_byte(cpustate->program,0x10000|Y),cpustate->a); cpustate->icount-=11;break;
	case 0x21: lh5801_sbc(cpustate,memory_read_byte(cpustate->program,0x10000|U)); cpustate->icount-=11;break;
	case 0x23: lh5801_adc(cpustate,memory_read_byte(cpustate->program,0x10000|U)); cpustate->icount-=11;break;
	case 0x25: lh5801_lda(cpustate,memory_read_byte(cpustate->program,0x10000|U)); cpustate->icount-=10;break;
	case 0x27: lh5801_cpa(cpustate,cpustate->a, memory_read_byte(cpustate->program,0x10000|U)); cpustate->icount-=11;break;
	case 0x28: X=U;cpustate->icount-=11;break;
	case 0x29: lh5801_and(cpustate,memory_read_byte(cpustate->program,0x10000|U)); cpustate->icount-=11;break;
	case 0x2a: lh5801_pop_word(cpustate,&cpustate->u); cpustate->icount-=15;break;
	case 0x2b: lh5801_ora(cpustate,memory_read_byte(cpustate->program,0x10000|U)); cpustate->icount-=11;break;
	case 0x2c: lh5801_dcs(cpustate,memory_read_byte(cpustate->program,0x10000|U)); cpustate->icount-=17; break;
	case 0x2d: lh5801_eor(cpustate,memory_read_byte(cpustate->program,0x10000|U)); cpustate->icount-=11;break;
	case 0x2e: memory_write_byte(cpustate->program,0x10000|U,cpustate->a); cpustate->icount-=10;break;
	case 0x2f: lh5801_bit(cpustate,memory_read_byte(cpustate->program,0x10000|U),cpustate->a); cpustate->icount-=11;break;
	case 0x40: lh5801_inc(cpustate,&XH);cpustate->icount-=9;break;
	case 0x42: lh5801_dec(cpustate,&XH);cpustate->icount-=9;break;
	case 0x48: X=S;cpustate->icount-=11;break;
	case 0x49: lh5801_and_mem(cpustate,0x10000|X, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=17;break;
	case 0x4a: X=X;cpustate->icount-=11;break; //!!!
	case 0x4b: lh5801_ora_mem(cpustate,0x10000|X, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=17;break;
	case 0x4c: cpustate->bf=0;/*off !*/ cpustate->icount-=8;break;
	case 0x4d: lh5801_bit(cpustate,memory_read_byte(cpustate->program,X|0x10000), memory_decrypted_read_byte(cpustate->program,P++));cpustate->icount-=14;break;
	case 0x4e: S=X;cpustate->icount-=11;break;
	case 0x4f: lh5801_add_mem(cpustate,0x10000|X, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=17;break;
	case 0x50: lh5801_inc(cpustate,&YH);cpustate->icount-=9;break;
	case 0x52: lh5801_dec(cpustate,&YH);cpustate->icount-=9;break;
	case 0x58: X=P;cpustate->icount-=11;break;
	case 0x59: lh5801_and_mem(cpustate,0x10000|Y, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=17;break;
	case 0x5a: Y=X;cpustate->icount-=11;break;
	case 0x5b: lh5801_ora_mem(cpustate,0x10000|Y, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=17;break;
	case 0x5d: lh5801_bit(cpustate,memory_read_byte(cpustate->program,Y|0x10000), memory_decrypted_read_byte(cpustate->program,P++));cpustate->icount-=14;break;
	case 0x5e: lh5801_jmp(cpustate,X);cpustate->icount-=11;break; // P=X
	case 0x5f: lh5801_add_mem(cpustate,0x10000|Y, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=17;break;
	case 0x60: lh5801_inc(cpustate,&UH);cpustate->icount-=9;break;
	case 0x62: lh5801_dec(cpustate,&UH);cpustate->icount-=9;break;
	case 0x69: lh5801_and_mem(cpustate,0x10000|U, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=17;break;
	case 0x6a: U=X;cpustate->icount-=11;break;
	case 0x6b: lh5801_ora_mem(cpustate,0x10000|U, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=17;break;
	case 0x6d: lh5801_bit(cpustate,memory_read_byte(cpustate->program,X|0x10000), memory_decrypted_read_byte(cpustate->program,P++));cpustate->icount-=14;break;
	case 0x6f: lh5801_add_mem(cpustate,0x10000|U, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=17;break;
	case 0x81: cpustate->t|=IE; /*sie !*/cpustate->icount-=8;break;
	case 0x88: lh5801_push_word(cpustate,X); cpustate->icount-=14;break;
	case 0x8a: lh5801_pop(cpustate); cpustate->icount-=12; break;
	case 0x8c: lh5801_dca(cpustate,memory_read_byte(cpustate->program,0x10000|X)); cpustate->icount-=19; break;
	case 0x8e: /*cdv clears internal devider*/cpustate->icount-=8;break;
	case 0x98: lh5801_push_word(cpustate,Y); cpustate->icount-=14;break;
	case 0x9c: lh5801_dca(cpustate,memory_read_byte(cpustate->program,0x10000|Y)); cpustate->icount-=19; break;
	case 0xa1: lh5801_sbc(cpustate,memory_read_byte(cpustate->program,0x10000|lh5801_readop_word(cpustate))); cpustate->icount-=17;break;
	case 0xa3: lh5801_adc(cpustate,memory_read_byte(cpustate->program,0x10000|lh5801_readop_word(cpustate))); cpustate->icount-=17;break;
	case 0xa5: lh5801_lda(cpustate,memory_read_byte(cpustate->program,0x10000|lh5801_readop_word(cpustate))); cpustate->icount-=16;break;
	case 0xa7: lh5801_cpa(cpustate,cpustate->a, memory_read_byte(cpustate->program,0x10000|lh5801_readop_word(cpustate))); cpustate->icount-=17;break;
	case 0xa8: lh5801_push_word(cpustate,U); cpustate->icount-=14;break;
	case 0xa9: lh5801_and(cpustate,memory_read_byte(cpustate->program,0x10000|lh5801_readop_word(cpustate))); cpustate->icount-=17;break;
	case 0xaa: lh5801_lda(cpustate,cpustate->t); cpustate->icount-=9;break;
	case 0xab: lh5801_ora(cpustate,memory_read_byte(cpustate->program,0x10000|lh5801_readop_word(cpustate))); cpustate->icount-=17;break;
	case 0xac: lh5801_dca(cpustate,memory_read_byte(cpustate->program,0x10000|U)); cpustate->icount-=19; break;
	case 0xad: lh5801_eor(cpustate,memory_read_byte(cpustate->program,0x10000|lh5801_readop_word(cpustate))); cpustate->icount-=17;break;
	case 0xae: memory_write_byte(cpustate->program,0x10000|lh5801_readop_word(cpustate),cpustate->a); cpustate->icount-=16;break;
	case 0xaf: lh5801_bit(cpustate,memory_read_byte(cpustate->program,0x10000|lh5801_readop_word(cpustate)),cpustate->a); cpustate->icount-=17;break;
	case 0xb1: /*hlt*/cpustate->icount-=8;break;
	case 0xba: lh5801_ita(cpustate);cpustate->icount-=9;break;
	case 0xbe: cpustate->t&=~IE; /*rie !*/cpustate->icount-=8;break;
	case 0xc0: cpustate->dp=0; /*rdp !*/cpustate->icount-=8;break;
	case 0xc1: cpustate->dp=1; /*sdp !*/cpustate->icount-=8;break;
	case 0xc8: lh5801_push(cpustate,cpustate->a); cpustate->icount-=11;break;
	case 0xca: lh5801_adr(cpustate,&cpustate->x);cpustate->icount-=11;break;
	case 0xcc: /*atp sends a to data bus*/cpustate->icount-=9;break;
	case 0xce: lh5801_am(cpustate,cpustate->a); cpustate->icount-=9; break;
	case 0xd3: lh5801_drr(cpustate,0x10000|X); cpustate->icount-=16; break;
	case 0xd7: lh5801_drl(cpustate,0x10000|X); cpustate->icount-=16; break;
	case 0xda: lh5801_adr(cpustate,&cpustate->y);cpustate->icount-=11;break;
	case 0xde: lh5801_am(cpustate,cpustate->a|0x100); cpustate->icount-=9; break;
	case 0xea: lh5801_adr(cpustate,&cpustate->u);cpustate->icount-=11;break;
	case 0xe9:
		adr=lh5801_readop_word(cpustate)|0x10000;
		lh5801_and_mem(cpustate,adr, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=23;
		break;
	case 0xeb:
		adr=lh5801_readop_word(cpustate)|0x10000;
		lh5801_ora_mem(cpustate,adr, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=23;
		break;
	case 0xec: cpustate->t=cpustate->a; cpustate->icount-=9;break;
	case 0xed:
		adr=lh5801_readop_word(cpustate)|0x10000;lh5801_bit(cpustate,memory_read_byte(cpustate->program,adr), memory_decrypted_read_byte(cpustate->program,P++));
		cpustate->icount-=20;break;
	case 0xef:
		adr=lh5801_readop_word(cpustate)|0x10000;
		lh5801_add_mem(cpustate,adr, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=23;
		break;

	default:
		logerror("lh5801 illegal opcode at %.4x fd%.2x\n",P-2, oper);
	}
}

static void lh5801_instruction(lh5801_state *cpustate)
{
	int oper;
	int adr;

	oper=memory_decrypted_read_byte(cpustate->program,P++);
	switch (oper) {
	case 0x00: lh5801_sbc(cpustate,XL); cpustate->icount-=6;break;
	case 0x01: lh5801_sbc(cpustate,memory_read_byte(cpustate->program,X)); cpustate->icount-=7;break;
	case 0x02: lh5801_adc(cpustate,XL); cpustate->icount-=6;break;
	case 0x03: lh5801_adc(cpustate,memory_read_byte(cpustate->program,X)); cpustate->icount-=7;break;
	case 0x04: lh5801_lda(cpustate,XL); cpustate->icount-=5;break;
	case 0x05: lh5801_lda(cpustate,memory_read_byte(cpustate->program,X)); cpustate->icount-=6;break;
	case 0x06: lh5801_cpa(cpustate,cpustate->a, XL); cpustate->icount-=6;break;
	case 0x07: lh5801_cpa(cpustate,cpustate->a, memory_read_byte(cpustate->program,X)); cpustate->icount-=7;break;
	case 0x08: XH=cpustate->a; cpustate->icount-=5; break;
	case 0x09: lh5801_and(cpustate,memory_read_byte(cpustate->program,X)); cpustate->icount-=7;break;
	case 0x0a: XL=cpustate->a; cpustate->icount-=5; break;
	case 0x0b: lh5801_ora(cpustate,memory_read_byte(cpustate->program,X)); cpustate->icount-=7;break;
	case 0x0c: lh5801_dcs(cpustate,memory_read_byte(cpustate->program,X)); cpustate->icount-=13; break;
	case 0x0d: lh5801_eor(cpustate,memory_read_byte(cpustate->program,X)); cpustate->icount-=7;break;
	case 0x0e: memory_write_byte(cpustate->program,X,cpustate->a); cpustate->icount-=6;break;
	case 0x0f: lh5801_bit(cpustate,memory_read_byte(cpustate->program,X),cpustate->a); cpustate->icount-=7;break;
	case 0x10: lh5801_sbc(cpustate,YL); cpustate->icount-=6;break;
	case 0x11: lh5801_sbc(cpustate,memory_read_byte(cpustate->program,Y)); cpustate->icount-=7;break;
	case 0x12: lh5801_adc(cpustate,YL); cpustate->icount-=6;break;
	case 0x13: lh5801_adc(cpustate,memory_read_byte(cpustate->program,Y)); cpustate->icount-=7;break;
	case 0x14: lh5801_lda(cpustate,YL); cpustate->icount-=5;break;
	case 0x15: lh5801_lda(cpustate,memory_read_byte(cpustate->program,Y)); cpustate->icount-=6;break;
	case 0x16: lh5801_cpa(cpustate,cpustate->a, YL); cpustate->icount-=6;break;
	case 0x17: lh5801_cpa(cpustate,cpustate->a, memory_read_byte(cpustate->program,Y)); cpustate->icount-=7;break;
	case 0x18: YH=cpustate->a; cpustate->icount-=5; break;
	case 0x19: lh5801_and(cpustate,memory_read_byte(cpustate->program,Y)); cpustate->icount-=7;break;
	case 0x1a: YL=cpustate->a; cpustate->icount-=5; break;
	case 0x1b: lh5801_ora(cpustate,memory_read_byte(cpustate->program,Y)); cpustate->icount-=7;break;
	case 0x1c: lh5801_dcs(cpustate,memory_read_byte(cpustate->program,Y)); cpustate->icount-=13; break;
	case 0x1d: lh5801_eor(cpustate,memory_read_byte(cpustate->program,Y)); cpustate->icount-=7;break;
	case 0x1e: memory_write_byte(cpustate->program,Y,cpustate->a); cpustate->icount-=6;break;
	case 0x1f: lh5801_bit(cpustate,memory_read_byte(cpustate->program,Y),cpustate->a); cpustate->icount-=7;break;
	case 0x20: lh5801_sbc(cpustate,UL); cpustate->icount-=6;break;
	case 0x21: lh5801_sbc(cpustate,memory_read_byte(cpustate->program,U)); cpustate->icount-=7;break;
	case 0x22: lh5801_adc(cpustate,UL); cpustate->icount-=6;break;
	case 0x23: lh5801_adc(cpustate,memory_read_byte(cpustate->program,U)); cpustate->icount-=7;break;
	case 0x24: lh5801_lda(cpustate,UL); cpustate->icount-=5;break;
	case 0x25: lh5801_lda(cpustate,memory_read_byte(cpustate->program,U)); cpustate->icount-=6;break;
	case 0x26: lh5801_cpa(cpustate,cpustate->a, UL); cpustate->icount-=6;break;
	case 0x27: lh5801_cpa(cpustate,cpustate->a, memory_read_byte(cpustate->program,U)); cpustate->icount-=7;break;
	case 0x28: UH=cpustate->a; cpustate->icount-=5; break;
	case 0x29: lh5801_and(cpustate,memory_read_byte(cpustate->program,U)); cpustate->icount-=7;break;
	case 0x2a: UL=cpustate->a; cpustate->icount-=5; break;
	case 0x2b: lh5801_ora(cpustate,memory_read_byte(cpustate->program,U)); cpustate->icount-=7;break;
	case 0x2c: lh5801_dcs(cpustate,memory_read_byte(cpustate->program,U)); cpustate->icount-=13; break;
	case 0x2d: lh5801_eor(cpustate,memory_read_byte(cpustate->program,U)); cpustate->icount-=7;break;
	case 0x2e: memory_write_byte(cpustate->program,U,cpustate->a); cpustate->icount-=6;break;
	case 0x2f: lh5801_bit(cpustate,memory_read_byte(cpustate->program,U),cpustate->a); cpustate->icount-=7;break;
	case 0x38: /*nop*/cpustate->icount-=5;break;
	case 0x40: lh5801_inc(cpustate,&XL);cpustate->icount-=5;break;
	case 0x41: lh5801_sin(cpustate,&cpustate->x); cpustate->icount-=6;break;
	case 0x42: lh5801_dec(cpustate,&XL);cpustate->icount-=5;break;
	case 0x43: lh5801_sde(cpustate,&cpustate->x); cpustate->icount-=6;break;
	case 0x44: X++;cpustate->icount-=5;break;
	case 0x45: lh5801_lin(cpustate,&cpustate->x);cpustate->icount-=6;break;
	case 0x46: X--;cpustate->icount-=5;break;
	case 0x47: lh5801_lde(cpustate,&cpustate->x);cpustate->icount-=6;break;
	case 0x48: XH=memory_decrypted_read_byte(cpustate->program,P++);cpustate->icount-=6;break;
	case 0x49: lh5801_and_mem(cpustate,X, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=13;break;
	case 0x4a: XL=memory_decrypted_read_byte(cpustate->program,P++);cpustate->icount-=6;break;
	case 0x4b: lh5801_ora_mem(cpustate,X, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=13;break;
	case 0x4c: lh5801_cpa(cpustate,XH, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0x4d: lh5801_bit(cpustate,memory_read_byte(cpustate->program,X), memory_decrypted_read_byte(cpustate->program,P++));cpustate->icount-=10;break;
	case 0x4e: lh5801_cpa(cpustate,XL, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0x4f: lh5801_add_mem(cpustate,X, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=13;break;
	case 0x50: lh5801_inc(cpustate,&YL);cpustate->icount-=5;break;
	case 0x51: lh5801_sin(cpustate,&cpustate->y); cpustate->icount-=6;break;
	case 0x52: lh5801_dec(cpustate,&YL);cpustate->icount-=5;break;
	case 0x53: lh5801_sde(cpustate,&cpustate->y); cpustate->icount-=6;break;
	case 0x54: Y++;cpustate->icount-=5;break;
	case 0x55: lh5801_lin(cpustate,&cpustate->y);cpustate->icount-=6;break;
	case 0x56: Y--;cpustate->icount-=5;break;
	case 0x57: lh5801_lde(cpustate,&cpustate->y);cpustate->icount-=6;break;
	case 0x58: YH=memory_decrypted_read_byte(cpustate->program,P++);cpustate->icount-=6;break;
	case 0x59: lh5801_and_mem(cpustate,Y, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=13;break;
	case 0x5a: YL=memory_decrypted_read_byte(cpustate->program,P++);cpustate->icount-=6;break;
	case 0x5b: lh5801_ora_mem(cpustate,Y, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=13;break;
	case 0x5c: lh5801_cpa(cpustate,YH, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0x5d: lh5801_bit(cpustate,memory_read_byte(cpustate->program,Y), memory_decrypted_read_byte(cpustate->program,P++));cpustate->icount-=10;break;
	case 0x5e: lh5801_cpa(cpustate,YL, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0x5f: lh5801_add_mem(cpustate,Y, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=13;break;
	case 0x60: lh5801_inc(cpustate,&UL);cpustate->icount-=5;break;
	case 0x61: lh5801_sin(cpustate,&cpustate->u); cpustate->icount-=6;break;
	case 0x62: lh5801_dec(cpustate,&UL);cpustate->icount-=5;break;
	case 0x63: lh5801_sde(cpustate,&cpustate->u); cpustate->icount-=6;break;
	case 0x64: U++;cpustate->icount-=5;break;
	case 0x65: lh5801_lin(cpustate,&cpustate->u);cpustate->icount-=6;break;
	case 0x66: U--;cpustate->icount-=5;break;
	case 0x67: lh5801_lde(cpustate,&cpustate->u);cpustate->icount-=6;break;
	case 0x68: UH=memory_decrypted_read_byte(cpustate->program,P++);cpustate->icount-=6;break;
	case 0x69: lh5801_and_mem(cpustate,U, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=13;break;
	case 0x6a: UL=memory_decrypted_read_byte(cpustate->program,P++);cpustate->icount-=6;break;
	case 0x6b: lh5801_ora_mem(cpustate,U, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=13;break;
	case 0x6c: lh5801_cpa(cpustate,UH, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0x6d: lh5801_bit(cpustate,memory_read_byte(cpustate->program,U), memory_decrypted_read_byte(cpustate->program,P++));cpustate->icount-=10;break;
	case 0x6e: lh5801_cpa(cpustate,UL, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0x6f: lh5801_add_mem(cpustate,U, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=13;break;
	case 0x80: lh5801_sbc(cpustate,XH); cpustate->icount-=6;break;
	case 0x81: lh5801_branch_plus(cpustate,!(cpustate->t&C)); cpustate->icount-=8; break;
	case 0x82: lh5801_adc(cpustate,XH); cpustate->icount-=6;break;
	case 0x83: lh5801_branch_plus(cpustate,cpustate->t&C); cpustate->icount-=8; break;
	case 0x84: lh5801_lda(cpustate,XH); cpustate->icount-=5;break;
	case 0x85: lh5801_branch_plus(cpustate,!(cpustate->t&H)); cpustate->icount-=8; break;
	case 0x86: lh5801_cpa(cpustate,cpustate->a, XH); cpustate->icount-=6;break;
	case 0x87: lh5801_branch_plus(cpustate,cpustate->t&H); cpustate->icount-=8; break;
	case 0x88: lh5801_lop(cpustate); break;
	case 0x89: lh5801_branch_plus(cpustate,!(cpustate->t&Z)); cpustate->icount-=8; break;
	case 0x8a: lh5801_rti(cpustate); cpustate->icount-=14; break;
	case 0x8b: lh5801_branch_plus(cpustate,cpustate->t&Z); cpustate->icount-=8; break;
	case 0x8c: lh5801_dca(cpustate,memory_read_byte(cpustate->program,X)); cpustate->icount-=15; break;
	case 0x8d: lh5801_branch_plus(cpustate,!(cpustate->t&V)); cpustate->icount-=8; break;
	case 0x8e: lh5801_branch_plus(cpustate,1); cpustate->icount-=5; break;
	case 0x8f: lh5801_branch_plus(cpustate,cpustate->t&V); cpustate->icount-=8; break;
	case 0x90: lh5801_sbc(cpustate,YH); cpustate->icount-=6;break;
	case 0x91: lh5801_branch_minus(cpustate,!(cpustate->t&C)); cpustate->icount-=8; break;
	case 0x92: lh5801_adc(cpustate,YH); cpustate->icount-=6;break;
	case 0x93: lh5801_branch_minus(cpustate,cpustate->t&C); cpustate->icount-=8; break;
	case 0x94: lh5801_lda(cpustate,YH); cpustate->icount-=5;break;
	case 0x95: lh5801_branch_minus(cpustate,!(cpustate->t&H)); cpustate->icount-=8; break;
	case 0x96: lh5801_cpa(cpustate,cpustate->a, YH); cpustate->icount-=6;break;
	case 0x97: lh5801_branch_minus(cpustate,cpustate->t&H); cpustate->icount-=8; break;
	case 0x99: lh5801_branch_minus(cpustate,!(cpustate->t&Z)); cpustate->icount-=8; break;
	case 0x9a: lh5801_rtn(cpustate); cpustate->icount-=11; break;
	case 0x9b: lh5801_branch_minus(cpustate,cpustate->t&Z); cpustate->icount-=8; break;
	case 0x9c: lh5801_dca(cpustate,memory_read_byte(cpustate->program,Y)); cpustate->icount-=15; break;
	case 0x9d: lh5801_branch_minus(cpustate,!(cpustate->t&V)); cpustate->icount-=8; break;
	case 0x9e: lh5801_branch_minus(cpustate,1); cpustate->icount-=6; break;
	case 0x9f: lh5801_branch_minus(cpustate,cpustate->t&V); cpustate->icount-=8; break;
	case 0xa0: lh5801_sbc(cpustate,UH); cpustate->icount-=6;break;
	case 0xa2: lh5801_adc(cpustate,UH); cpustate->icount-=6;break;
	case 0xa1: lh5801_sbc(cpustate,memory_read_byte(cpustate->program,lh5801_readop_word(cpustate))); cpustate->icount-=13;break;
	case 0xa3: lh5801_adc(cpustate,memory_read_byte(cpustate->program,lh5801_readop_word(cpustate))); cpustate->icount-=13;break;
	case 0xa4: lh5801_lda(cpustate,UH); cpustate->icount-=5;break;
	case 0xa5: lh5801_lda(cpustate,memory_read_byte(cpustate->program,lh5801_readop_word(cpustate))); cpustate->icount-=12;break;
	case 0xa6: lh5801_cpa(cpustate,cpustate->a, UH); cpustate->icount-=6;break;
	case 0xa7: lh5801_cpa(cpustate,cpustate->a, memory_read_byte(cpustate->program,lh5801_readop_word(cpustate))); cpustate->icount-=13;break;
	case 0xa8: cpustate->pv=1;/*spv!*/ cpustate->icount-=4; break;
	case 0xa9: lh5801_and(cpustate,memory_read_byte(cpustate->program,lh5801_readop_word(cpustate))); cpustate->icount-=13;break;
	case 0xaa: S=lh5801_readop_word(cpustate);cpustate->icount-=6;break;
	case 0xab: lh5801_ora(cpustate,memory_read_byte(cpustate->program,lh5801_readop_word(cpustate))); cpustate->icount-=13;break;
	case 0xac: lh5801_dca(cpustate,memory_read_byte(cpustate->program,U)); cpustate->icount-=15; break;
	case 0xad: lh5801_eor(cpustate,memory_read_byte(cpustate->program,lh5801_readop_word(cpustate))); cpustate->icount-=13;break;
	case 0xae: memory_write_byte(cpustate->program,lh5801_readop_word(cpustate),cpustate->a); cpustate->icount-=12;break;
	case 0xaf: lh5801_bit(cpustate,memory_read_byte(cpustate->program,lh5801_readop_word(cpustate)),cpustate->a); cpustate->icount-=13;break;
	case 0xb1: lh5801_sbc(cpustate,memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0xb3: lh5801_adc(cpustate,memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0xb5: lh5801_lda(cpustate,memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=6;break;
	case 0xb7: lh5801_cpa(cpustate,cpustate->a, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0xb8: cpustate->pv=0;/*rpv!*/ cpustate->icount-=4; break;
	case 0xb9: lh5801_and(cpustate,memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0xba: lh5801_jmp(cpustate,lh5801_readop_word(cpustate)); cpustate->icount-=12;break;
	case 0xbb: lh5801_ora(cpustate,memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0xbd: lh5801_eor(cpustate,memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0xbe: lh5801_sjp(cpustate); cpustate->icount-=19; break;
	case 0xbf: lh5801_bit(cpustate,cpustate->a, memory_decrypted_read_byte(cpustate->program,P++));cpustate->icount-=7;break;
	case 0xc1: lh5801_vector(cpustate,!(cpustate->t&C), memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=8;break;
	case 0xc3: lh5801_vector(cpustate,cpustate->t&C, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=8;break;
	case 0xc5: lh5801_vector(cpustate,!(cpustate->t&H), memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=8;break;
	case 0xc7: lh5801_vector(cpustate,cpustate->t&H, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=8;break;
	case 0xc9: lh5801_vector(cpustate,!(cpustate->t&Z), memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=8;break;
	case 0xcb: lh5801_vector(cpustate,cpustate->t&Z, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=8;break;
	case 0xcd: lh5801_vector(cpustate,1, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=7;break;
	case 0xcf: lh5801_vector(cpustate,cpustate->t&V, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=8;break;
	case 0xd1: lh5801_ror(cpustate); cpustate->icount-=6; break;
	case 0xd3: lh5801_drr(cpustate,X); cpustate->icount-=12; break;
	case 0xd5: lh5801_shr(cpustate); cpustate->icount-=6; break;
	case 0xd7: lh5801_drl(cpustate,X); cpustate->icount-=12; break;
	case 0xd9: lh5801_shl(cpustate); cpustate->icount-=6; break;
	case 0xdb: lh5801_rol(cpustate); cpustate->icount-=6; break;
	case 0xdd: lh5801_inc(cpustate,&cpustate->a);cpustate->icount-=5;break;
	case 0xdf: lh5801_dec(cpustate,&cpustate->a);cpustate->icount-=5;break;
	case 0xe1: cpustate->pu=1;/*spu!*/ cpustate->icount-=4; break;
	case 0xe3: cpustate->pu=0;/*rpu!*/ cpustate->icount-=4; break;
	case 0xe9:
		adr=lh5801_readop_word(cpustate);lh5801_and_mem(cpustate,adr, memory_decrypted_read_byte(cpustate->program,P++));
		cpustate->icount-=19;break;
	case 0xeb:
		adr=lh5801_readop_word(cpustate);lh5801_ora_mem(cpustate,adr, memory_decrypted_read_byte(cpustate->program,P++));
		cpustate->icount-=19;break;
	case 0xed:
		adr=lh5801_readop_word(cpustate);lh5801_bit(cpustate,memory_read_byte(cpustate->program,adr), memory_decrypted_read_byte(cpustate->program,P++));
		cpustate->icount-=16;break;
	case 0xef:
		adr=lh5801_readop_word(cpustate);
		lh5801_add_mem(cpustate,adr, memory_decrypted_read_byte(cpustate->program,P++)); cpustate->icount-=19;
		break;
	case 0xf1: lh5801_aex(cpustate); cpustate->icount-=6; break;
	case 0xf5: memory_write_byte(cpustate->program,Y++, memory_read_byte(cpustate->program,X++)); cpustate->icount-=7; break; //tin
	case 0xf7: lh5801_cpa(cpustate,cpustate->a, memory_read_byte(cpustate->program,X++));cpustate->icount-=7; break; //cin
	case 0xf9: cpustate->t&=~C;cpustate->icount-=4;break;
	case 0xfb: cpustate->t|=C;cpustate->icount-=4;break;
	case 0xfd: lh5801_instruction_fd(cpustate);break;
	case 0xc0: case 0xc2: case 0xc4: case 0xc6:
	case 0xc8: case 0xca: case 0xcc: case 0xce:
	case 0xd0: case 0xd2: case 0xd4: case 0xd6:
	case 0xd8: case 0xda: case 0xdc: case 0xde:
	case 0xe0: case 0xe2: case 0xe4: case 0xe6:
	case 0xe8: case 0xea: case 0xec: case 0xee:
	case 0xf0: case 0xf2: case 0xf4: case 0xf6:
		lh5801_vector(cpustate,1, oper);cpustate->icount-=4;break;
	default:
		logerror("lh5801 illegal opcode at %.4x %.2x\n",P-1, oper);
	}

}

