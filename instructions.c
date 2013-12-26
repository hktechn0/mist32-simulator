#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <err.h>

#include "common.h"
#include "interrupt.h"
#include "instructions.h"
#include "operands.h"
#include "flags.h"

/* Arithmetic */
void i_add(Instruction insn)
{
  int *dest;
  int src, result;
  
  ops_o2_i11(insn, &dest, &src);
  
  result = (*dest) + src;
  set_flags_add(result, *dest, src);
  *dest = result;
}

inline void i_sub(Instruction insn)
{
  int *dest;
  int src, result;
  
  ops_o2_i11(insn, &dest, &src);
  
  result = (*dest) - src;
  set_flags_sub(result, *dest, src);
  *dest = result;
}

void i_mull(Instruction insn)
{
  int *dest;
  int src;

  ops_o2_i11(insn, &dest, &src);
  
  *dest = *dest * src;

  clr_flags();
  /* FIXME: flags unavailable */
}

void i_mulh(Instruction insn)
{ 
  int *dest;
  int src;
  long long result;

  ops_o2_i11(insn, &dest, &src);

  result = (long long)(*dest) * (long long)src;
  *dest = result >> 32;

  clr_flags();
  /* FIXME: flags unavailable */
}

void i_udiv(Instruction insn)
{
  unsigned int *dest;
  unsigned int src;

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

void i_umod(Instruction insn)
{
  unsigned int *dest;
  unsigned int src;

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

void i_cmp(Instruction insn)
{
  int dest;
  
  dest = GR[insn.o2.operand1];
  i_sub(insn);
  GR[insn.o2.operand1] = dest;
}

void i_div(Instruction insn)
{
  int *dest;
  int src;

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

void i_mod(Instruction insn)
{
  int *dest;
  int src;

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

void i_neg(Instruction insn)
{
  GR[insn.o2.operand1] = -GR[insn.o2.operand2];
  set_flags(GR[insn.o2.operand1]);
  if(GR[insn.o2.operand1] == 0x80000000) { FLAGR.overflow = 1; }
}

void i_addc(Instruction insn)
{
  int *dest;
  int src, result;
  
  ops_o2_i11(insn, &dest, &src);
  
  result = (*dest) + src;
  set_flags_add(result, *dest, src);
  *dest = FLAGR.carry;
}

void i_inc(Instruction insn)
{
  int *dest;
  int src, result;
  
  dest = &GR[insn.o2.operand1];
  src = GR[insn.o2.operand2];
  
  result = src + 1;
  set_flags_add(result, src, 1);
  *dest = result;
}

void i_dec(Instruction insn)
{
  int *dest;
  int src, result;
  
  dest = &GR[insn.o2.operand1];
  src = GR[insn.o2.operand2];
  
  result = src - 1;
  set_flags_sub(result, src, 1);
  *dest = result;
}

void i_sext8(Instruction insn)
{
  unsigned int *dest, src;

  dest = (unsigned int *)&GR[insn.o2.operand1];
  src = (unsigned int)GR[insn.o2.operand2];

  *dest = SIGN_EXT8(src);
}

void i_sext16(Instruction insn)
{
  unsigned int *dest, src;

  dest = (unsigned int *)&GR[insn.o2.operand1];
  src = (unsigned int)GR[insn.o2.operand2];

  *dest = SIGN_EXT16(src);
}

/* Shift, Rotate */
void i_shl(Instruction insn)
{
  unsigned int *dest, n;
  
  ops_o2_ui11(insn, &dest, &n);
  
  clr_flags();
  FLAGR.carry = (*dest) >> (32 - n);
  *dest = (*dest) << n;
  set_flags(*dest);
}

void i_shr(Instruction insn)
{
  unsigned int *dest, n;
  
  ops_o2_ui11(insn, &dest, &n);
  
  clr_flags();
  FLAGR.carry = (*dest >> (n - 1)) & 0x00000001;
  *dest = (*dest) >> n;
  set_flags(*dest);
}

void i_sar(Instruction insn)
{
  int *dest;
  unsigned int n;

  ops_o2_i11(insn, &dest, (int *)&n);

  clr_flags();
  FLAGR.carry = ((*dest) >> (n - 1)) & 0x00000001;
  *dest = (*dest) >> n;
  set_flags(*dest);
}

void i_rol(Instruction insn)
{
  unsigned int *dest, n;
  
  ops_o2_ui11(insn, &dest, &n);
  
  *dest = ((*dest) << n) | ((*dest) >> (32 - n));
  
  clr_flags();
  set_flags(*dest);
  FLAGR.carry = !!(*dest & 0x00000001);
}

void i_ror(Instruction insn)
{
  unsigned int *dest, n;
  
  ops_o2_ui11(insn, &dest, &n);
  
  *dest = ((*dest) << (32 - n));
  
  clr_flags();
  set_flags(*dest);
  FLAGR.carry = !!(*dest & 0x80000000);
}

/* Logic */
inline void i_and(Instruction insn)
{
  GR[insn.o2.operand1] &= GR[insn.o2.operand2];
  clr_flags();
  set_flags(GR[insn.o2.operand1]);
}

inline void i_or(Instruction insn)
{
  GR[insn.o2.operand1] |= GR[insn.o2.operand2];
  clr_flags();
  set_flags(GR[insn.o2.operand1]);
}

void i_not(Instruction insn)
{
  GR[insn.o2.operand1] = ~GR[insn.o2.operand2];
}

inline void i_xor(Instruction insn)
{
  GR[insn.o2.operand1] ^= GR[insn.o2.operand2];
  clr_flags();
  set_flags(GR[insn.o2.operand1]);
}

void i_nand(Instruction insn)
{
  i_and(insn);
  NOT(GR[insn.o2.operand1]);

  clr_flags();
  set_flags(GR[insn.o2.operand1]);
}

void i_nor(Instruction insn)
{
  i_or(insn);
  NOT(GR[insn.o2.operand1]);

  clr_flags();
  set_flags(GR[insn.o2.operand1]);
}

void i_xnor(Instruction insn)
{
  i_xor(insn);
  NOT(GR[insn.o2.operand1]);

  clr_flags();
  set_flags(GR[insn.o2.operand1]);
}

void i_test(Instruction insn)
{
  int dest;
  
  dest = GR[insn.o2.operand1];
  i_and(insn);
  GR[insn.o2.operand1] = dest;
}

/* Register operations */
void i_wl16(Instruction insn)
{
  GR[insn.i16.operand] = ((unsigned int)GR[insn.i16.operand] & 0xffff0000) | (immediate_i16(insn) & 0xffff);
}

void i_wh16(Instruction insn)
{
  GR[insn.i16.operand] = ((unsigned int)GR[insn.i16.operand] & 0xffff) | ((immediate_i16(insn) & 0xffff) << 16);
}

void i_clrb(Instruction insn)
{
  GR[insn.i11.operand] &= ~((unsigned int)0x01 << immediate_i11(insn));
}

void i_setb(Instruction insn)
{
  GR[insn.i11.operand] |= (unsigned int)0x01 << immediate_i11(insn);
}

void i_clr(Instruction insn)
{
  GR[insn.o1.operand1] = 0x00000000;
}

void i_set(Instruction insn)
{
  GR[insn.o1.operand1] = (unsigned int)0xffffffff;
}

void i_revb(Instruction insn)
{
  /* FIXME: not implement */
  fprintf(stderr, "[Error] %s not implemented yet.\n", "revb");
  exit(EXIT_FAILURE);
}

void i_rev8(Instruction insn)
{
  unsigned int *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  *dest = __builtin_bswap32(src);
}

void i_getb(Instruction insn)
{
  /* FIXME: not implement */
  fprintf(stderr, "[Error] %s not implemented yet.\n", "getb");
  exit(EXIT_FAILURE);
}

void i_get8(Instruction insn)
{
  unsigned int *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  *dest = ((*dest) >> (src * 4)) & 0xff;
}

void i_lil(Instruction insn)
{
  GR[insn.i16.operand] = immediate_i16(insn);
}

void i_lih(Instruction insn)
{
  GR[insn.i16.operand] = (unsigned int)immediate_ui16(insn) << 16;
}

void i_ulil(Instruction insn)
{
  GR[insn.i16.operand] = immediate_ui16(insn);
}

/* Load, Store */
void i_ld8(Instruction insn)
{
  unsigned int *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(memory_ld8(dest, src)) {
    /* memory fault */
    return;
  }

  DEBUGLD("[Load ] Addr: 0x%08x, Data:       0x%02x, PC: 0x%08x\n", src, *dest, PCR);
  DEBUGLDHW("[L], %08x, %08x, %08x, %08x\n", PCR, SPR, src, *dest);
}

void i_ld16(Instruction insn)
{
  unsigned int *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(insn.i11.is_immediate) {
    src <<= 1;
  }
  else if(src & 0x1) {
    abort_sim();
    errx(EXIT_FAILURE, "ld16: invalid alignment.");
  }

  if(memory_ld16(dest, src)) {
    /* memory fault */
    return;
  }

  DEBUGLD("[Load ] Addr: 0x%08x, Data:     0x%04x, PC: 0x%08x\n", src, *dest, PCR);
  DEBUGLDHW("[L], %08x, %08x, %08x, %08x\n", PCR, SPR, src, *dest);
}

void i_ld32(Instruction insn)
{
  unsigned int *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(insn.i11.is_immediate) {
    src <<= 2;
  }
  else if(src & 0x3) {
    abort_sim();
    errx(EXIT_FAILURE, "ld32: invalid alignment.");
  }

  if(memory_ld32(dest, src)) {
    /* memory fault */
    return;
  }

  DEBUGLD("[Load ] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", src, *dest, PCR);
  DEBUGLDHW("[L], %08x, %08x, %08x, %08x\n", PCR, SPR, src, *dest);
}

void i_st8(Instruction insn)
{
  unsigned int *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(memory_st8(src, (unsigned char)*dest)) {
    /* memory fault */
    return;
  }

  DEBUGST("[Store] Addr: 0x%08x, Data:       0x%02x, PC: 0x%08x\n", src, (unsigned char)*dest, PCR);
  DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, src, (unsigned char)*dest);
}

void i_st16(Instruction insn)
{
  unsigned int *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(insn.i11.is_immediate) {
    src <<= 1;
  }
  else if(src & 0x1) {
    abort_sim();
    errx(EXIT_FAILURE, "st16: invalid alignment.");
  }

  if(memory_st16(src, (unsigned short)*dest)) {
    /* memory fault */
    return;
  }

  DEBUGST("[Store] Addr: 0x%08x, Data:     0x%04x, PC: 0x%08x\n", src, (unsigned short)*dest, PCR);
  DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, src, (unsigned short)*dest);
}

void i_st32(Instruction insn)
{
  unsigned int *dest, src;

  ops_o2_ui11(insn, &dest, &src);

  if(insn.i11.is_immediate) {
    src <<= 2;
  }
  else if(src & 0x3) {
    abort_sim();
    errx(EXIT_FAILURE, "st32: invalid alignment.");
  }

  if(memory_st32(src, *dest)) {
    /* memory fault */
    return;
  }

  DEBUGST("[Store] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", src, *dest, PCR);
  DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, src, *dest);
}

/* Stack */
void i_push(Instruction insn)
{
  unsigned int src;

  SPR -= 4;
  src = insn.c.is_immediate ? insn.c.immediate : GR[insn.o1.operand1];

  if(memory_st32(SPR, src)) {
    /* memory fault */
    return;
  }

  DEBUGST("[Push] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", SPR, src, PCR);
  DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, SPR, src);
}

void i_pushpc(Instruction insn)
{
  SPR -= 4;

  if(memory_st32(SPR, PCR)) {
    /* memory fault */
    return;
  }
}

void i_pop(Instruction insn)
{
  unsigned int *dest;

  dest = (unsigned int *)&(GR[insn.o1.operand1]);

  if(memory_ld32(dest, SPR)) {
    /* memory fault */
    return;
  }

  DEBUGLD("[Pop] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", SPR, *dest, PCR);
  DEBUGLDHW("[L], %08x, %08x, %08x, %08x\n", PCR, SPR, SPR, *dest);

  SPR += 4;
}

/* Branch */
void i_bur(Instruction insn)
{
  if(check_condition(insn)) {
    DEBUGJMP("[Branch] URE: 0x%08x, Cond: %X, PC: 0x%08x\n", PCR + src_jo1_jui16(insn), insn.ji16.condition, PCR);
    next_PCR = PCR + src_jo1_jui16(insn);
  }
}

void i_br(Instruction insn)
{
  if(check_condition(insn)) {
    DEBUGJMP("[Branch] REL: 0x%08x, Cond: %X, PC: 0x%08x\n", PCR + src_jo1_ji16(insn), insn.ji16.condition, PCR);
    next_PCR = PCR + src_jo1_ji16(insn);
  }
}

void i_b(Instruction insn)
{
  if(check_condition(insn)) {
    DEBUGJMP("[Branch]  D : 0x%08x, Cond: %X, PCR: 0x%08x\n", src_jo1_jui16(insn), insn.ji16.condition, PCR);
    next_PCR = src_jo1_jui16(insn);
  }

  /* instruction_prefetch_flush(); */

  /* for traceback */
  if(!insn.jo1.is_immediate && insn.jo1.operand1 == GR_RET) {
    if(traceback > 0) {
      traceback_next--;
    }
  }
}

void i_ib(Instruction insn)
{
  interrupt_exit();
}

void i_srspr(Instruction insn)
{
  GR[insn.o1.operand1] = SPR;
}

void i_srpdtr(Instruction insn)
{
  GR[insn.o1.operand1] = PDTR;
}

void i_srpidr(Instruction insn)
{
  /* FIXME: not implemented */
  GR[insn.o1.operand1] = 0;
}

void i_srcidr(Instruction insn)
{
  /* FIXME: not implemented */
  GR[insn.o1.operand1] = 0;
}

void i_srmoder(Instruction insn)
{
  /* FIXME: not implemented */
  GR[insn.o1.operand1] = 0;
}

void i_srieir(Instruction insn)
{
  GR[insn.o1.operand1] = (PSR & PSR_IM_ENABLE) >> 2;
}

void i_srmmur(Instruction insn)
{
  GR[insn.o1.operand1] = (PSR & PSR_MMUMOD_MASK);
}

void i_sriosr(Instruction insn)
{
  GR[insn.o1.operand1] = IOSR;
}

void i_srtidr(Instruction insn)
{
  GR[insn.o1.operand1] = TIDR;
}

void i_srppsr(Instruction insn)
{
  GR[insn.o1.operand1] = PPSR;
}

void i_srppcr(Instruction insn)
{
  GR[insn.o1.operand1] = PPCR;
}

void i_sruspr(Instruction insn)
{
  GR[insn.o1.operand1] = USPR;
}

void i_srppdtr(Instruction insn)
{
  GR[insn.o1.operand1] = PPDTR;
}

void i_srptidr(Instruction insn)
{
  GR[insn.o1.operand1] = PTIDR;
}

void i_srpsr(Instruction insn)
{
  GR[insn.o1.operand1] = PSR;
}

void i_srfrcr(Instruction insn)
{
  FRCR = (unsigned long long)clock();
}

void i_srfrclr(Instruction insn)
{
  GR[insn.o1.operand1] = (unsigned int)(FRCR & 0xffffffff);
}

void i_srfrchr(Instruction insn)
{
  GR[insn.o1.operand1] = (unsigned int)(FRCR >> 32);
}

void i_srpflagr(Instruction insn)
{
  GR[insn.o1.operand1] = PFLAGR.flags;
}

void i_srspw(Instruction insn)
{
  SPR = GR[insn.o1.operand1];
}

void i_srpdtw(Instruction insn)
{
  PDTR = GR[insn.o1.operand1];
  DEBUGMMU("[MMU] SRPDTW: 0x%08x\n", PDTR);

  memory_tlb_flush();
  instruction_prefetch_flush();
}

void i_srieiw(Instruction insn)
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

void i_srmmuw(Instruction insn)
{
  PSR = (PSR & ~PSR_MMUMOD_MASK) | (src_o1_i11(insn) & PSR_MMUMOD_MASK);
  DEBUGMMU("[MMU] SRMMUW: MMUMOD %d\n", PSR & PSR_MMUMOD_MASK);

  memory_tlb_flush();
  instruction_prefetch_flush();
}

void i_srppsw(Instruction insn)
{
  PPSR = (Memory)GR[insn.o1.operand1];
}

void i_srppcw(Instruction insn)
{
  PPCR = (Memory)GR[insn.o1.operand1];

  if(PPCR & 0x3) {
    errx(EXIT_FAILURE, "srppcw: invalid alignment. %08x PC: %08x", PPCR, PCR);
  }
}

void i_sruspw(Instruction insn)
{
  USPR = (Memory)GR[insn.o1.operand1];
}

void i_srppdtw(Instruction insn)
{
  PPDTR = (Memory)GR[insn.o1.operand1];
}

void i_srptidw(Instruction insn)
{
  PTIDR = (Memory)GR[insn.o1.operand1];
}

void i_sridtw(Instruction insn)
{
  IDTR = (Memory)GR[insn.o1.operand1];
  DEBUGINT("[INTERRUPT] SRIDTW: idtr <= 0x%08x\n", IDTR);
}

void i_srpsw(Instruction insn)
{
  if((GR[insn.o1.operand1] & PSR_MMUMOD_MASK) != (PSR & PSR_MMUMOD_MASK)) {
    memory_tlb_flush();
    instruction_prefetch_flush();
  }

  PSR = GR[insn.o1.operand1];
  DEBUGMMU("[MMU] SRPSW: MMUMOD %d MMUPS %d\n", PSR_MMUMOD, PSR_MMUPS);

  if(PSR_MMUPS != PSR_MMUPS_4KB) {
    abort_sim();
    errx(EXIT_FAILURE, "MMU page size (%d) not supported.", PSR_MMUPS);
  }
}

void i_srpflagw(Instruction insn)
{
  PFLAGR.flags = GR[insn.o1.operand1];
}

void i_srspadd(Instruction insn)
{
  SPR += (int)SIGN_EXT16(insn.c.immediate << 2);
}

void i_nop(Instruction insn)
{
  return;
}

void i_halt(Instruction insn)
{
  exit(EXIT_FAILURE);
}

void i_move(Instruction insn)
{
  GR[insn.o2.operand1] = GR[insn.o2.operand2];
}

void i_movepc(Instruction insn)
{
  int *dest, src;

  ops_o2_i11(insn, &dest, &src);
  *dest = PCR + (src << 2);

  /* for traceback */
  if(insn.o2.operand1 == GR_RET) {
    if(traceback_next < TRACEBACK_MAX) {
      traceback[traceback_next++] = PCR + 4;
    }
  }
}

void i_swi(Instruction insn)
{
  unsigned int num;

  num = immediate_i11(insn);

  if(num < IDT_SWIRQ_START_NUM) {
    /* software irq number should be 64 or above */
    errx(EXIT_FAILURE, "swi: invalid swi (%d)", num);
  }

  interrupt_dispatch_nonmask(num);
}

void i_tas(Instruction insn)
{
  unsigned int *dest, src;

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

void i_idts(Instruction insn)
{
  interrupt_idt_store();
}
