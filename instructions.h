static inline void debug_load_hw(Memory addr, unsigned int data)
{
  DEBUGLDHW("[L], %08x, %08x, %08x, %08x\n", PCR, SPR, addr, data);
  DEBUGLDPHY("[L], %08x(%08x), %08x(%08x), %08x(%08x), %08x\n",
	     PCR, memory_addr_virt2phy(PCR, false, false),
	     SPR, memory_addr_virt2phy(SPR, false, false),
	     addr, memory_addr_virt2phy(addr, false, false), data);
}

static inline void debug_store_hw(Memory addr, unsigned int data)
{
  DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, addr, data);
  DEBUGSTPHY("[S], %08x(%08x), %08x(%08x), %08x(%08x), %08x\n",
	     PCR, memory_addr_virt2phy(PCR, false, false),
	     SPR, memory_addr_virt2phy(SPR, false, false),
	     addr, memory_addr_virt2phy(addr, false, false), data);
}

static inline void debug_load8(Memory addr, unsigned char data)
{
  DEBUGLD("[Load ] Addr: 0x%08x, Data:       0x%02x, PC: 0x%08x\n", addr, data, PCR);
  debug_load_hw(addr, data);
}

static inline void debug_load16(Memory addr, unsigned short data)
{
  DEBUGLD("[Load ] Addr: 0x%08x, Data:     0x%04x, PC: 0x%08x\n", addr, data, PCR);
  debug_load_hw(addr, data);
}

static inline void debug_load32(Memory addr, unsigned int data)
{
  DEBUGLD("[Load ] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", addr, data, PCR);
  debug_load_hw(addr, data);
}

static inline void debug_store8(Memory addr, unsigned char data)
{
  DEBUGST("[Store] Addr: 0x%08x, Data:       0x%02x, PC: 0x%08x\n", addr, data, PCR);
  debug_store_hw(addr, data);
}

static inline void debug_store16(Memory addr, unsigned short data)
{
  DEBUGST("[Store] Addr: 0x%08x, Data:     0x%04x, PC: 0x%08x\n", addr, data, PCR);
  debug_store_hw(addr, data);
}

static inline void debug_store32(Memory addr, unsigned int data)
{
  DEBUGST("[Store] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", addr, data, PCR);
  debug_store_hw(addr, data);
}

void i_add(const Instruction insn);
void i_sub(const Instruction insn);
void i_mull(const Instruction insn);
void i_mulh(const Instruction insn);
void i_udiv(const Instruction insn);
void i_umod(const Instruction insn);
void i_cmp(const Instruction insn);
void i_div(const Instruction insn);
void i_mod(const Instruction insn);
void i_neg(const Instruction insn);

void i_addc(const Instruction insn);
void i_inc(const Instruction insn);
void i_dec(const Instruction insn);
void i_sext8(const Instruction insn);
void i_sext16(const Instruction insn);

void i_shl(const Instruction insn);
void i_shr(const Instruction insn);
void i_sar(const Instruction insn);
void i_rol(const Instruction insn);
void i_ror(const Instruction insn);

void i_and(const Instruction insn);
void i_or(const Instruction insn);
void i_not(const Instruction insn);
void i_xor(const Instruction insn);
void i_nand(const Instruction insn);
void i_nor(const Instruction insn);
void i_xnor(const Instruction insn);
void i_test(const Instruction insn);

void i_wl16(const Instruction insn);
void i_wh16(const Instruction insn);
void i_clrb(const Instruction insn);
void i_setb(const Instruction insn);
void i_clr(const Instruction insn);
void i_set(const Instruction insn);
void i_revb(const Instruction insn);
void i_rev8(const Instruction insn);
void i_getb(const Instruction insn);
void i_get8(const Instruction insn);

void i_lil(const Instruction insn);
void i_lih(const Instruction insn);
void i_ulil(const Instruction insn);

void i_ld8(const Instruction insn);
void i_ld16(const Instruction insn);
void i_ld32(const Instruction insn);
void i_st8(const Instruction insn);
void i_st16(const Instruction insn);
void i_st32(const Instruction insn);

void i_push(const Instruction insn);
void i_pushpc(const Instruction insn);
void i_pop(const Instruction insn);

void i_bur(const Instruction insn);
void i_br(const Instruction insn);
void i_b(const Instruction insn);
void i_ib(const Instruction insn);

void i_srspr(const Instruction insn);
void i_srpdtr(const Instruction insn);
void i_srpidr(const Instruction insn);
void i_srcidr(const Instruction insn);
void i_srmoder(const Instruction insn);
void i_srieir(const Instruction insn);
/* void i_srtisr(const Instruction insn); */
void i_srkpdtr(const Instruction insn);
void i_srmmur(const Instruction insn);
void i_sriosr(const Instruction insn);
void i_srtidr(const Instruction insn);
void i_srppsr(const Instruction insn);
void i_srppcr(const Instruction insn);
void i_sruspr(const Instruction insn);
void i_srppdtr(const Instruction insn);
void i_srptidr(const Instruction insn);
void i_srpsr(const Instruction insn);
void i_srfrcr(const Instruction insn);
void i_srfrclr(const Instruction insn);
void i_srfrchr(const Instruction insn);
void i_srpflagr(const Instruction insn);
void i_srfi0r(const Instruction insn);
void i_srfi1r(const Instruction insn);

void i_srspw(const Instruction insn);
void i_srpdtw(const Instruction insn);
void i_srieiw(const Instruction insn);
/* void i_srtisw(const Instruction insn); */
void i_srkpdtw(const Instruction insn);
void i_srmmuw(const Instruction insn);
void i_srppsw(const Instruction insn);
void i_srppcw(const Instruction insn);
void i_sruspw(const Instruction insn);
void i_srppdtw(const Instruction insn);
void i_srptidw(const Instruction insn);
void i_sridtw(const Instruction insn);
void i_srpsw(const Instruction insn);
/* void i_srfrcw(const Instruction insn); */
/* void i_srfrclw(const Instruction insn); */
/* void i_srfrchw(const Instruction insn); */
void i_srpflagw(const Instruction insn); /* ??? */
void i_srspadd(const Instruction insn);

void i_nop(const Instruction insn);
void i_halt(const Instruction insn);
void i_move(const Instruction insn);
void i_movepc(const Instruction insn);

void i_swi(const Instruction insn);
void i_tas(const Instruction insn);
void i_idts(const Instruction insn);
