void i_add(Instruction *inst);
void i_sub(Instruction *inst);
void i_mull(Instruction *inst);
void i_mulh(Instruction *inst);
void i_udiv(Instruction *inst);
void i_umod(Instruction *inst);
void i_cmp(Instruction *inst);
void i_div(Instruction *inst);
void i_mod(Instruction *inst);
void i_neg(Instruction *inst);

void i_addc(Instruction *inst);
void i_inc(Instruction *inst);
void i_dec(Instruction *inst);
void i_sext8(Instruction *inst);
void i_sext16(Instruction *inst);

void i_shl(Instruction *inst);
void i_shr(Instruction *inst);
void i_sar(Instruction *inst);
void i_rol(Instruction *inst);
void i_ror(Instruction *inst);

void i_and(Instruction *inst);
void i_or(Instruction *inst);
void i_not(Instruction *inst);
void i_xor(Instruction *inst);
void i_nand(Instruction *inst);
void i_nor(Instruction *inst);
void i_xnor(Instruction *inst);
void i_test(Instruction *inst);

void i_wl16(Instruction *inst);
void i_wh16(Instruction *inst);
void i_clrb(Instruction *inst);
void i_setb(Instruction *inst);
void i_clr(Instruction *inst);
void i_set(Instruction *inst);
void i_revb(Instruction *inst);
void i_rev8(Instruction *inst);
void i_getb(Instruction *inst);
void i_get8(Instruction *inst);

void i_lil(Instruction *inst);
void i_lih(Instruction *inst);
void i_ulil(Instruction *inst);

void i_ld8(Instruction *inst);
void i_ld16(Instruction *inst);
void i_ld32(Instruction *inst);
void i_st8(Instruction *inst);
void i_st16(Instruction *inst);
void i_st32(Instruction *inst);

void i_push(Instruction *inst);
void i_pushpc(Instruction *inst);
void i_pop(Instruction *inst);

void i_bur(Instruction *inst);
void i_br(Instruction *inst);
void i_b(Instruction *inst);
void i_ib(Instruction *inst);

void i_srspr(Instruction *inst);
void i_srieir(Instruction *inst);
void i_sriosr(Instruction *inst);
void i_sridtr(Instruction *inst);

void i_srspw(Instruction *inst);
void i_srieiw(Instruction *inst);
void i_sridtw(Instruction *inst);

void i_nop(Instruction *inst);
void i_halt(Instruction *inst);
void i_move(Instruction *inst);
void i_movepc(Instruction *inst);
