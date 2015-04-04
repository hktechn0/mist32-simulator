#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <err.h>

#include "operands.h"
#include "flags.h"
#include "insn_debug.h"

/* Arithmetic */
void i_add(const Instruction insn)
{
  int32_t *dest;
  int32_t src, result;
  
  ops_o2_i11(insn, &dest, &src);
  
  result = (*dest) + src;
  set_flags_add(result, *dest, src);
  *dest = result;
}

void i_sub(const Instruction insn)
{
  int32_t *dest;
  int32_t src, result;
  
  ops_o2_i11(insn, &dest, &src);
  
  result = (*dest) - src;
  set_flags_sub(result, *dest, src);
  *dest = result;
}

void i_mull(const Instruction insn)
{
  int32_t *dest;
  int32_t src;

  ops_o2_i11(insn, &dest, &src);
  
  *dest = *dest * src;

  clr_flags();
  /* FIXME: flags unavailable */
}

void i_mulh(const Instruction insn)
{ 
  int32_t *dest;
  int32_t src;
  int64_t result;

  ops_o2_i11(insn, &dest, &src);

  result = (int64_t)(*dest) * (int64_t)src;
  *dest = result >> 32;

  clr_flags();
  /* FIXME: flags unavailable */
}

void i_udiv(const Instruction insn)
{
  uint32_t *dest;
  uint32_t src;

  ops_o2_ui11(insn, &dest, &src);

  if(src == 0) {
    DEBUGINT("[INTERRUPT] Zero division error. PC: %08x\n", PCR);
    interrupt_dispatch_nonmask(IDT_DIVERROR_NUM);
    return;
  }

  *dest = *dest / src;
  
  clr_flags();
  /* FIXME: flags unavailable */
}

void i_umod(const Instruction insn)
{
  uint32_t *dest;
  uint32_t src;

  ops_o2_ui11(insn, &dest, &src);

  if(src == 0) {
    DEBUGINT("[INTERRUPT] Zero division error. PC: %08x\n", PCR);
    interrupt_dispatch_nonmask(IDT_DIVERROR_NUM);
    return;
  }

  *dest = *dest % src;
  
  clr_flags();
  /* FIXME: flags unavailable */
}

void i_cmp(const Instruction insn)
{
  int32_t *dest;
  int32_t src, result;

  ops_o2_i11(insn, &dest, &src);

  result = (*dest) - src;
  set_flags_sub(result, *dest, src);
}

void i_div(const Instruction insn)
{
  int32_t *dest;
  int32_t src;

  ops_o2_i11(insn, &dest, &src);

  if(src == 0) {
    DEBUGINT("[INTERRUPT] Zero division error. PC: %08x\n", PCR);
    interrupt_dispatch_nonmask(IDT_DIVERROR_NUM);
    return;
  }

  *dest = *dest / src;

  clr_flags();
  /* FIXME: flags unavailable */
}

void i_mod(const Instruction insn)
{
  int32_t *dest;
  int32_t src;

  ops_o2_i11(insn, &dest, &src);

  if(src == 0) {
    DEBUGINT("[INTERRUPT] Zero division error. PC: %08x\n", PCR);
    interrupt_dispatch_nonmask(IDT_DIVERROR_NUM);
    return;
  }

  *dest = *dest % src;
  
  clr_flags();
  /* FIXME: flags unavailable */
}

void i_neg(const Instruction insn)
{
  GR[insn.o2.operand1] = -GR[insn.o2.operand2];
  set_flags(GR[insn.o2.operand1]);
  if(GR[insn.o2.operand1] == 0x80000000) { FLAGR.overflow = 1; }
}

void i_addc(const Instruction insn)
{
  int32_t *dest;
  int32_t src, result;
  
  ops_o2_i11(insn, &dest, &src);
  
  result = (*dest) + src;
  set_flags_add(result, *dest, src);
  *dest = FLAGR.carry;
}

void i_inc(const Instruction insn)
{
  int32_t *dest;
  int32_t src, result;
  
  dest = &GR[insn.o2.operand1];
  src = GR[insn.o2.operand2];
  
  result = src + 1;
  set_flags_add(result, src, 1);
  *dest = result;
}

void i_dec(const Instruction insn)
{
  int32_t *dest;
  int32_t src, result;
  
  dest = &GR[insn.o2.operand1];
  src = GR[insn.o2.operand2];
  
  result = src - 1;
  set_flags_sub(result, src, 1);
  *dest = result;
}

void i_sext8(const Instruction insn)
{
  GR[insn.o2.operand1] = SIGN_EXT8((uint32_t)GR[insn.o2.operand2]);
}

void i_sext16(const Instruction insn)
{
  GR[insn.o2.operand1] = SIGN_EXT16((uint32_t)GR[insn.o2.operand2]);
}

/* Shift, Rotate */
void i_shl(const Instruction insn)
{
  uint32_t *dest, n;
  
  ops_o2_ui11(insn, &dest, &n);
  
  clr_flags();
  FLAGR.carry = (*dest) >> (32 - n);
  *dest = (*dest) << n;
  set_flags(*dest);
}

void i_shr(const Instruction insn)
{
  uint32_t *dest, n;
  
  ops_o2_ui11(insn, &dest, &n);
  
  clr_flags();
  FLAGR.carry = (*dest >> (n - 1)) & 0x00000001;
  *dest = (*dest) >> n;
  set_flags(*dest);
}

void i_sar(const Instruction insn)
{
  int32_t *dest;
  uint32_t n;

  ops_o2_i11(insn, &dest, (int *)&n);

  clr_flags();
  FLAGR.carry = ((*dest) >> (n - 1)) & 0x00000001;
  *dest = (*dest) >> n;
  set_flags(*dest);
}

void i_rol(const Instruction insn)
{
  uint32_t *dest, n;
  
  ops_o2_ui11(insn, &dest, &n);
  
  *dest = ((*dest) << n) | ((*dest) >> (32 - n));
  
  clr_flags();
  set_flags(*dest);
  FLAGR.carry = !!(*dest & 0x00000001);
}

void i_ror(const Instruction insn)
{
  uint32_t *dest, n;
  
  ops_o2_ui11(insn, &dest, &n);
  
  *dest = ((*dest) << (32 - n));
  
  clr_flags();
  set_flags(*dest);
  FLAGR.carry = !!(*dest & 0x80000000);
}

/* Logic */
void i_and(const Instruction insn)
{
  GR[insn.o2.operand1] &= GR[insn.o2.operand2];
  clr_flags();
  set_flags(GR[insn.o2.operand1]);
}

void i_or(const Instruction insn)
{
  GR[insn.o2.operand1] |= GR[insn.o2.operand2];
  clr_flags();
  set_flags(GR[insn.o2.operand1]);
}

void i_not(const Instruction insn)
{
  GR[insn.o2.operand1] = ~GR[insn.o2.operand2];
}

void i_xor(const Instruction insn)
{
  GR[insn.o2.operand1] ^= GR[insn.o2.operand2];
  clr_flags();
  set_flags(GR[insn.o2.operand1]);
}

void i_nand(const Instruction insn)
{
  i_and(insn);
  NOT(GR[insn.o2.operand1]);

  clr_flags();
  set_flags(GR[insn.o2.operand1]);
}

void i_nor(const Instruction insn)
{
  i_or(insn);
  NOT(GR[insn.o2.operand1]);

  clr_flags();
  set_flags(GR[insn.o2.operand1]);
}

void i_xnor(const Instruction insn)
{
  i_xor(insn);
  NOT(GR[insn.o2.operand1]);

  clr_flags();
  set_flags(GR[insn.o2.operand1]);
}

void i_test(const Instruction insn)
{
  int32_t dest;
  
  dest = GR[insn.o2.operand1];
  i_and(insn);
  GR[insn.o2.operand1] = dest;
}

/* Register operations */
void i_wl16(const Instruction insn)
{
  GR[insn.i16.operand] = ((uint32_t)GR[insn.i16.operand] & 0xffff0000) | (immediate_i16(insn) & 0xffff);
}

void i_wh16(const Instruction insn)
{
  GR[insn.i16.operand] = ((uint32_t)GR[insn.i16.operand] & 0xffff) | ((immediate_i16(insn) & 0xffff) << 16);
}

void i_clrb(const Instruction insn)
{
  GR[insn.i11.operand] &= ~((uint32_t)0x01 << immediate_i11(insn));
}

void i_setb(const Instruction insn)
{
  GR[insn.i11.operand] |= (uint32_t)0x01 << immediate_i11(insn);
}

void i_clr(const Instruction insn)
{
  GR[insn.o1.operand1] = 0x00000000;
}

void i_set(const Instruction insn)
{
  GR[insn.o1.operand1] = (uint32_t)0xffffffff;
}

void i_revb(const Instruction insn)
{
  /* FIXME: not implement */
  fprintf(stderr, "[Error] %s not implemented yet.\n", "revb");
  exit(EXIT_FAILURE);
}

void i_rev8(const Instruction insn)
{
  uint32_t *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  *dest = __builtin_bswap32(src);
}

void i_getb(const Instruction insn)
{
  /* FIXME: not implement */
  fprintf(stderr, "[Error] %s not implemented yet.\n", "getb");
  exit(EXIT_FAILURE);
}

void i_get8(const Instruction insn)
{
  uint32_t *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  *dest = ((*dest) >> (src * 4)) & 0xff;
}

void i_lil(const Instruction insn)
{
  GR[insn.i16.operand] = immediate_i16(insn);
}

void i_lih(const Instruction insn)
{
  GR[insn.i16.operand] = (uint32_t)immediate_ui16(insn) << 16;
}

void i_ulil(const Instruction insn)
{
  GR[insn.i16.operand] = immediate_ui16(insn);
}

/* Load, Store */
void i_ld8(const Instruction insn)
{
  uint32_t *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(!insn.i11.is_immediate) {
    src += (int)SIGN_EXT6(insn.o2.displacement);
  }

  if(memory_ld8(dest, src)) {
    /* memory fault */
    return;
  }

  debug_load16(src, (unsigned char)*dest);
}

void i_ld16(const Instruction insn)
{
  uint32_t *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(insn.i11.is_immediate) {
    src <<= 1;
  }
  else if(src & 0x1) {
    abort_sim();
    errx(EXIT_FAILURE, "ld16: invalid alignment.");
  }
  else {
    src += (int)(SIGN_EXT6(insn.o2.displacement) << 1);
  }

  if(memory_ld16(dest, src)) {
    /* memory fault */
    return;
  }

  debug_load16(src, (unsigned short)*dest);
}

void i_ld32(const Instruction insn)
{
  uint32_t *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(insn.i11.is_immediate) {
    src <<= 2;
  }
  else if(src & 0x3) {
    abort_sim();
    errx(EXIT_FAILURE, "ld32: invalid alignment.");
  }
  else {
    src += (int)(SIGN_EXT6(insn.o2.displacement) << 2);
  }

  if(memory_ld32(dest, src)) {
    /* memory fault */
    return;
  }

  debug_load32(src, *dest);
}

void i_st8(const Instruction insn)
{
  uint32_t *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(!insn.i11.is_immediate) {
    src += (int)SIGN_EXT6(insn.o2.displacement);
  }

  if(memory_st8(src, (unsigned char)*dest)) {
    /* memory fault */
    return;
  }

  debug_store8(src, (unsigned char)*dest);
}

void i_st16(const Instruction insn)
{
  uint32_t *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(insn.i11.is_immediate) {
    src <<= 1;
  }
  else if(src & 0x1) {
    abort_sim();
    errx(EXIT_FAILURE, "st16: invalid alignment.");
  }
  else {
    src += (int)(SIGN_EXT6(insn.o2.displacement) << 1);
  }

  if(memory_st16(src, (unsigned short)*dest)) {
    /* memory fault */
    return;
  }

  debug_store16(src, (unsigned short)*dest);
}

void i_st32(const Instruction insn)
{
  uint32_t *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(insn.i11.is_immediate) {
    src <<= 2;
  }
  else if(src & 0x3) {
    abort_sim();
    errx(EXIT_FAILURE, "st32: invalid alignment.");
  }
  else {
    src += (int)(SIGN_EXT6(insn.o2.displacement) << 2);
  }

  if(memory_st32(src, *dest)) {
    /* memory fault */
    return;
  }

  debug_store32(src, *dest);
}

/* Stack */
void i_push(const Instruction insn)
{
  uint32_t src;

  SPR -= 4;
  src = insn.c.is_immediate ? insn.c.immediate : GR[insn.o1.operand1];

  if(memory_st32(SPR, src)) {
    /* memory fault */
    return;
  }

  DEBUGST("[Push ] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", SPR, src, PCR);
  debug_store_hw(SPR, src);
}

void i_pushpc(const Instruction insn)
{
  SPR -= 4;

  if(memory_st32(SPR, PCR)) {
    /* memory fault */
    return;
  }
}

void i_pop(const Instruction insn)
{
  uint32_t *dest;

  dest = (uint32_t *)&(GR[insn.o1.operand1]);

  if(memory_ld32(dest, SPR)) {
    /* memory fault */
    return;
  }

  SPR += 4;

  DEBUGLD("[Pop  ] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", SPR - 4, *dest, PCR);
  debug_load_hw(SPR - 4, *dest);
}

/* Branch */
void i_bur(const Instruction insn)
{
  if(check_condition(insn)) {
    DEBUGJMP("[Branch] URE: 0x%08x, Cond: %X, PC: 0x%08x\n", PCR + src_jo1_jui16(insn), insn.ji16.condition, PCR);
    next_PCR = PCR + src_jo1_jui16(insn);
  }
}

void i_br(const Instruction insn)
{
  if(check_condition(insn)) {
    DEBUGJMP("[Branch] REL: 0x%08x, Cond: %X, PC: 0x%08x\n", PCR + src_jo1_ji16(insn), insn.ji16.condition, PCR);
    next_PCR = PCR + src_jo1_ji16(insn);
  }
}

void i_b(const Instruction insn)
{
  if(check_condition(insn)) {
    DEBUGJMP("[Branch]  D : 0x%08x, Cond: %X, PCR: 0x%08x\n", src_jo1_jui16(insn), insn.ji16.condition, PCR);
    next_PCR = src_jo1_jui16(insn);
  }

  /* instruction_prefetch_flush(); */

  /* for traceback */
#if !NO_DEBUG
  if(!insn.jo1.is_immediate && insn.jo1.operand1 == GR_RET) {
    if(traceback > 0) {
      traceback_next--;
    }
  }
#endif
}

void i_ib(const Instruction insn)
{
  interrupt_exit();
}

void i_srspr(const Instruction insn)
{
  GR[insn.o1.operand1] = SPR;
}

void i_srpdtr(const Instruction insn)
{
  GR[insn.o1.operand1] = PDTR;
}

void i_srkpdtr(const Instruction insn)
{
  GR[insn.o1.operand1] = KPDTR;
}

void i_srpidr(const Instruction insn)
{
  /* FIXME: not implemented */
  GR[insn.o1.operand1] = 0;
}

void i_srcidr(const Instruction insn)
{
  /* FIXME: not implemented */
  GR[insn.o1.operand1] = 0;
}

void i_srmoder(const Instruction insn)
{
  /* FIXME: not implemented */
  GR[insn.o1.operand1] = 0;
}

void i_srieir(const Instruction insn)
{
  GR[insn.o1.operand1] = (PSR & PSR_IM_ENABLE) >> 2;
}

void i_srmmur(const Instruction insn)
{
  GR[insn.o1.operand1] = (PSR & PSR_MMUMOD_MASK);
}

void i_sriosr(const Instruction insn)
{
  GR[insn.o1.operand1] = IOSR;
}

void i_srtidr(const Instruction insn)
{
  GR[insn.o1.operand1] = TIDR;
}

void i_srppsr(const Instruction insn)
{
  GR[insn.o1.operand1] = PPSR;
}

void i_srppcr(const Instruction insn)
{
  GR[insn.o1.operand1] = PPCR;
}

void i_sruspr(const Instruction insn)
{
  GR[insn.o1.operand1] = USPR;
}

void i_srppdtr(const Instruction insn)
{
  GR[insn.o1.operand1] = PPDTR;
}

void i_srptidr(const Instruction insn)
{
  GR[insn.o1.operand1] = PTIDR;
}

void i_srpsr(const Instruction insn)
{
  GR[insn.o1.operand1] = PSR;
}

void i_srfrcr(const Instruction insn)
{
  FRCR = (unsigned long long)clock();
}

void i_srfrclr(const Instruction insn)
{
  GR[insn.o1.operand1] = (uint32_t)(FRCR & 0xffffffff);
}

void i_srfrchr(const Instruction insn)
{
  GR[insn.o1.operand1] = (uint32_t)(FRCR >> 32);
}

void i_srpflagr(const Instruction insn)
{
  GR[insn.o1.operand1] = PFLAGR.flags;
}

void i_srfi0r(const Instruction insn)
{
  GR[insn.o1.operand1] = FI0R;
}

void i_srfi1r(const Instruction insn)
{
  GR[insn.o1.operand1] = FI1R;
}

void i_srspw(const Instruction insn)
{
  SPR = GR[insn.o1.operand1];
}

void i_srpdtw(const Instruction insn)
{
  PDTR = GR[insn.o1.operand1];
  DEBUGMMU("[MMU] SRPDTW: 0x%08x\n", PDTR);

  memory_tlb_flush();
  instruction_prefetch_flush();
}

void i_srkpdtw(const Instruction insn)
{
  KPDTR = GR[insn.o1.operand1];
  DEBUGMMU("[MMU] SRKPDTW: 0x%08x\n", KPDTR);

  memory_tlb_flush();
  instruction_prefetch_flush();
}

void i_srieiw(const Instruction insn)
{
  if(src_o1_i11(insn) & 1) {
    PSR |= PSR_IM_ENABLE;

    if(!(PSR & PSR_IM_ENABLE)) {
      DEBUGINT("[INTERRUPT] SRIEIW: Interrupt Enabled.\n");
    }
  }
  else {
    PSR &= ~PSR_IM_ENABLE;

    if(PSR & PSR_IM_ENABLE) {
      DEBUGINT("[INTERRUPT] SRIEIW: Interrupt Disabled.\n");
    }
  }
}

void i_srmmuw(const Instruction insn)
{
  PSR = (PSR & ~PSR_MMUMOD_MASK) | (src_o1_i11(insn) & PSR_MMUMOD_MASK);
  DEBUGMMU("[MMU] SRMMUW: MMUMOD %d\n", PSR & PSR_MMUMOD_MASK);

  memory_tlb_flush();
  instruction_prefetch_flush();
}

void i_srppsw(const Instruction insn)
{
  PPSR = (Memory)GR[insn.o1.operand1];
}

void i_srppcw(const Instruction insn)
{
  PPCR = (Memory)GR[insn.o1.operand1];

  if(PPCR & 0x3) {
    errx(EXIT_FAILURE, "srppcw: invalid alignment. %08x PC: %08x", PPCR, PCR);
  }
}

void i_sruspw(const Instruction insn)
{
  USPR = (Memory)GR[insn.o1.operand1];
}

void i_srppdtw(const Instruction insn)
{
  PPDTR = (Memory)GR[insn.o1.operand1];
}

void i_srptidw(const Instruction insn)
{
  PTIDR = (Memory)GR[insn.o1.operand1];
}

void i_sridtw(const Instruction insn)
{
  IDTR = (Memory)GR[insn.o1.operand1];
  DEBUGINT("[INTERRUPT] SRIDTW: idtr <= 0x%08x\n", IDTR);
}

void i_srpsw(const Instruction insn)
{
  if(((GR[insn.o1.operand1] & PSR_MMUMOD_MASK) != (PSR & PSR_MMUMOD_MASK)) ||
     ((GR[insn.o1.operand1] & PSR_CMOD_MASK) != (PSR & PSR_CMOD_MASK))) {
    memory_tlb_flush();
    instruction_prefetch_flush();
  }

  PSR = GR[insn.o1.operand1];
  DEBUGMMU("[MMU] SRPSW: MMUMOD %d MMUPS %d\n", PSR_MMUMOD, PSR_MMUPS);

  if(PSR_MMUMOD && PSR_MMUPS != PSR_MMUPS_4KB) {
    abort_sim();
    errx(EXIT_FAILURE, "MMU page size (%d) not supported.", PSR_MMUPS);
  }
}

void i_srpflagw(const Instruction insn)
{
  PFLAGR.flags = GR[insn.o1.operand1];
}

void i_srspadd(const Instruction insn)
{
  SPR += (int)SIGN_EXT16(insn.c.immediate << 2);
}

void i_nop(const Instruction insn)
{
  return;
}

void i_halt(const Instruction insn)
{
  exit(EXIT_FAILURE);
}

void i_move(const Instruction insn)
{
  GR[insn.o2.operand1] = GR[insn.o2.operand2];
}

void i_movepc(const Instruction insn)
{
  int32_t *dest, src;

  ops_o2_i11(insn, &dest, &src);
  *dest = PCR + (src << 2);

  /* for traceback */
#if !NO_DEBUG
  if(insn.o2.operand1 == GR_RET) {
    if(traceback_next < TRACEBACK_MAX) {
      traceback[traceback_next++] = PCR + 4;
    }
  }
#endif
}

void i_swi(const Instruction insn)
{
  unsigned int num;

  num = immediate_ui11(insn);

  if(num < IDT_SWIRQ_START_NUM) {
    /* software irq number should be 64 or above */
    errx(EXIT_FAILURE, "swi: invalid swi (%d)", num);
  }

  interrupt_dispatch_nonmask(num);
}

void i_tas(const Instruction insn)
{
  uint32_t *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(insn.i11.is_immediate) {
    src <<= 2;
  }
  else if(src & 0x3) {
    errx(EXIT_FAILURE, "tas: invalid alignment.");
  }

  /* load flag */
  if(memory_ld32(dest, src)) {
    /* memory fault */
    return;
  }

  /* test */
  if(*dest == 0) {
    /* success, and set */
    if(memory_st32(src, 1)) {
      /* memory fault */
      return;
    }
  }
  else {
    /* fail, do nothing */
  }

  DEBUGST("[TAS] Addr: 0x%08x, %s, PC: 0x%08x\n", src, *dest ? "fail" : "success", PCR);
  /* DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, src, *dest); */
}

void i_idts(const Instruction insn)
{
  interrupt_idt_store();
}

void i_invalid(const Instruction insn)
{
  print_instruction(insn);
  abort_sim();
  errx(EXIT_FAILURE, "invalid opcode. (pc:%08x op:%x)", PCR, insn.base.opcode);
}
