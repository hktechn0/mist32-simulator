void i_add(Instruction insn);
void i_sub(Instruction insn);
void i_mull(Instruction insn);
void i_mulh(Instruction insn);
void i_udiv(Instruction insn);
void i_umod(Instruction insn);
void i_cmp(Instruction insn);
void i_div(Instruction insn);
void i_mod(Instruction insn);
void i_neg(Instruction insn);

void i_addc(Instruction insn);
void i_inc(Instruction insn);
void i_dec(Instruction insn);
void i_sext8(Instruction insn);
void i_sext16(Instruction insn);

void i_shl(Instruction insn);
void i_shr(Instruction insn);
void i_sar(Instruction insn);
void i_rol(Instruction insn);
void i_ror(Instruction insn);

void i_and(Instruction insn);
void i_or(Instruction insn);
void i_not(Instruction insn);
void i_xor(Instruction insn);
void i_nand(Instruction insn);
void i_nor(Instruction insn);
void i_xnor(Instruction insn);
void i_test(Instruction insn);

void i_wl16(Instruction insn);
void i_wh16(Instruction insn);
void i_clrb(Instruction insn);
void i_setb(Instruction insn);
void i_clr(Instruction insn);
void i_set(Instruction insn);
void i_revb(Instruction insn);
void i_rev8(Instruction insn);
void i_getb(Instruction insn);
void i_get8(Instruction insn);

void i_lil(Instruction insn);
void i_lih(Instruction insn);
void i_ulil(Instruction insn);

void i_ld8(Instruction insn);
void i_ld16(Instruction insn);
void i_ld32(Instruction insn);
void i_st8(Instruction insn);
void i_st16(Instruction insn);
void i_st32(Instruction insn);

void i_push(Instruction insn);
void i_pushpc(Instruction insn);
void i_pop(Instruction insn);

void i_bur(Instruction insn);
void i_br(Instruction insn);
void i_b(Instruction insn);
void i_ib(Instruction insn);

void i_srspr(Instruction insn);
void i_srpdtr(Instruction insn);
void i_srpidr(Instruction insn);
void i_srcidr(Instruction insn);
void i_srmoder(Instruction insn);
void i_srieir(Instruction insn);
/* void i_srtisr(Instruction insn); */
/* void i_srkpdtr(Instruction insn); */
void i_srmmur(Instruction insn);
void i_sriosr(Instruction insn);
void i_srtidr(Instruction insn);
void i_srppsr(Instruction insn);
void i_srppcr(Instruction insn);
void i_sruspr(Instruction insn);
void i_srppdtr(Instruction insn);
void i_srptidr(Instruction insn);
void i_srpsr(Instruction insn);
void i_srfrcr(Instruction insn);
void i_srfrclr(Instruction insn);
void i_srfrchr(Instruction insn);
void i_srpflagr(Instruction insn);

void i_srspw(Instruction insn);
void i_srpdtw(Instruction insn);
void i_srieiw(Instruction insn);
/* void i_srtisw(Instruction insn); */
/* void i_srkpdtw(Instruction insn); */
void i_srmmuw(Instruction insn);
void i_srppsw(Instruction insn);
void i_srppcw(Instruction insn);
void i_sruspw(Instruction insn);
void i_srppdtw(Instruction insn);
void i_srptidw(Instruction insn);
void i_sridtw(Instruction insn);
void i_srpsw(Instruction insn);
/* void i_srfrcw(Instruction insn); */
/* void i_srfrclw(Instruction insn); */
/* void i_srfrchw(Instruction insn); */
void i_srpflagw(Instruction insn); /* ??? */
void i_srspadd(Instruction insn);

void i_nop(Instruction insn);
void i_halt(Instruction insn);
void i_move(Instruction insn);
void i_movepc(Instruction insn);

void i_swi(Instruction insn);
void i_tas(Instruction insn);
void i_idts(Instruction insn);
