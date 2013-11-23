#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include <time.h>
#include <endian.h>

#include "common.h"
#include "interrupt.h"
#include "instructions.h"
#include "operands.h"
#include "flags.h"

/* Arithmetic */
void i_add(Instruction *inst)
{
  int *dest;
  int src, result;
  
  ops_o2_i11(inst, &dest, &src);
  
  result = (*dest) + src;
  set_flags_add(result, *dest, src);
  *dest = result;
}

inline void i_sub(Instruction *inst)
{
  int *dest;
  int src, result;
  
  ops_o2_i11(inst, &dest, &src);
  
  result = (*dest) - src;
  set_flags_sub(result, *dest, src);
  *dest = result;
}

void i_mull(Instruction *inst)
{
  int *dest;
  int src;

  ops_o2_i11(inst, &dest, &src);
  
  *dest = *dest * src;

  clr_flags();
  /* FIXME: flags unavailable */
}

void i_mulh(Instruction *inst)
{ 
  int *dest;
  int src;
  long long result;

  ops_o2_i11(inst, &dest, &src);

  result = (long long)(*dest) * (long long)src;
  *dest = result >> 32;

  clr_flags();
  /* FIXME: flags unavailable */
}

void i_udiv(Instruction *inst)
{
  unsigned int *dest;
  unsigned int src;

  ops_o2_ui11(inst, &dest, &src);

  if(src == 0) {
    DEBUGINT("[INTERRUPT] Zero division error. PC: %08x\n", PCR);
    interrupt_dispatch_nonmask(IDT_DIVERROR_NUM);
    return;
  }

  *dest = *dest / src;
  
  clr_flags();
  /* FIXME: flags unavailable */
}

void i_umod(Instruction *inst)
{
  unsigned int *dest;
  unsigned int src;

  ops_o2_ui11(inst, &dest, &src);

  if(src == 0) {
    DEBUGINT("[INTERRUPT] Zero division error. PC: %08x\n", PCR);
    interrupt_dispatch_nonmask(IDT_DIVERROR_NUM);
    return;
  }

  *dest = *dest % src;
  
  clr_flags();
  /* FIXME: flags unavailable */
}

void i_cmp(Instruction *inst)
{
  int dest;
  
  dest = GR[inst->o2.operand1];
  i_sub(inst);
  GR[inst->o2.operand1] = dest;
}

void i_div(Instruction *inst)
{
  int *dest;
  int src;

  ops_o2_i11(inst, &dest, &src);

  if(src == 0) {
    DEBUGINT("[INTERRUPT] Zero division error. PC: %08x\n", PCR);
    interrupt_dispatch_nonmask(IDT_DIVERROR_NUM);
    return;
  }

  *dest = *dest / src;

  clr_flags();
  /* FIXME: flags unavailable */
}

void i_mod(Instruction *inst)
{
  int *dest;
  int src;

  ops_o2_i11(inst, &dest, &src);

  if(src == 0) {
    DEBUGINT("[INTERRUPT] Zero division error. PC: %08x\n", PCR);
    interrupt_dispatch_nonmask(IDT_DIVERROR_NUM);
    return;
  }

  *dest = *dest % src;
  
  clr_flags();
  /* FIXME: flags unavailable */
}

void i_neg(Instruction *inst)
{
  GR[inst->o2.operand1] = -GR[inst->o2.operand2];
  set_flags(GR[inst->o2.operand1]);
  if(GR[inst->o2.operand1] == 0x80000000) { FLAGR.overflow = 1; }
}

void i_addc(Instruction *inst)
{
  int *dest;
  int src, result;
  
  ops_o2_i11(inst, &dest, &src);
  
  result = (*dest) + src;
  set_flags_add(result, *dest, src);
  *dest = FLAGR.carry;
}

void i_inc(Instruction *inst)
{
  int *dest;
  int src, result;
  
  dest = &GR[inst->o2.operand1];
  src = GR[inst->o2.operand2];
  
  result = src + 1;
  set_flags_add(result, src, 1);
  *dest = result;
}

void i_dec(Instruction *inst)
{
  int *dest;
  int src, result;
  
  dest = &GR[inst->o2.operand1];
  src = GR[inst->o2.operand2];
  
  result = src - 1;
  set_flags_sub(result, src, 1);
  *dest = result;
}

void i_sext8(Instruction *inst)
{
  unsigned int *dest, src;

  dest = (unsigned int *)&GR[inst->o2.operand1];
  src = (unsigned int)GR[inst->o2.operand2];

  *dest = SIGN_EXT8(src);
}

void i_sext16(Instruction *inst)
{
  unsigned int *dest, src;

  dest = (unsigned int *)&GR[inst->o2.operand1];
  src = (unsigned int)GR[inst->o2.operand2];

  *dest = SIGN_EXT16(src);
}

/* Shift, Rotate */
void i_shl(Instruction *inst)
{
  unsigned int *dest, n;
  
  ops_o2_ui11(inst, &dest, &n);
  
  clr_flags();
  FLAGR.carry = (*dest) >> (32 - n);
  *dest = (*dest) << n;
  set_flags(*dest);
}

void i_shr(Instruction *inst)
{
  unsigned int *dest, n;
  
  ops_o2_ui11(inst, &dest, &n);
  
  clr_flags();
  FLAGR.carry = (*dest >> (n - 1)) & 0x00000001;
  *dest = (*dest) >> n;
  set_flags(*dest);
}

void i_sar(Instruction *inst)
{
  int *dest;
  unsigned int n;

  ops_o2_i11(inst, &dest, (int *)&n);

  clr_flags();
  FLAGR.carry = ((*dest) >> (n - 1)) & 0x00000001;
  *dest = (*dest) >> n;
  set_flags(*dest);
}

void i_rol(Instruction *inst)
{
  unsigned int *dest, n;
  
  ops_o2_ui11(inst, &dest, &n);
  
  *dest = ((*dest) << n) | ((*dest) >> (32 - n));
  
  clr_flags();
  set_flags(*dest);
  FLAGR.carry = !!(*dest & 0x00000001);
}

void i_ror(Instruction *inst)
{
  unsigned int *dest, n;
  
  ops_o2_ui11(inst, &dest, &n);
  
  *dest = ((*dest) << (32 - n));
  
  clr_flags();
  set_flags(*dest);
  FLAGR.carry = !!(*dest & 0x80000000);
}

/* Logic */
inline void i_and(Instruction *inst)
{
  GR[inst->o2.operand1] &= GR[inst->o2.operand2];
  clr_flags();
  set_flags(GR[inst->o2.operand1]);
}

inline void i_or(Instruction *inst)
{
  GR[inst->o2.operand1] |= GR[inst->o2.operand2];
  clr_flags();
  set_flags(GR[inst->o2.operand1]);
}

void i_not(Instruction *inst)
{
  GR[inst->o2.operand1] = ~GR[inst->o2.operand2];
}

inline void i_xor(Instruction *inst)
{
  GR[inst->o2.operand1] ^= GR[inst->o2.operand2];
  clr_flags();
  set_flags(GR[inst->o2.operand1]);
}

void i_nand(Instruction *inst)
{
  i_and(inst);
  NOT(GR[inst->o2.operand1]);

  clr_flags();
  set_flags(GR[inst->o2.operand1]);
}

void i_nor(Instruction *inst)
{
  i_or(inst);
  NOT(GR[inst->o2.operand1]);

  clr_flags();
  set_flags(GR[inst->o2.operand1]);
}

void i_xnor(Instruction *inst)
{
  i_xor(inst);
  NOT(GR[inst->o2.operand1]);

  clr_flags();
  set_flags(GR[inst->o2.operand1]);
}

void i_test(Instruction *inst)
{
  int dest;
  
  dest = GR[inst->o2.operand1];
  i_and(inst);
  GR[inst->o2.operand1] = dest;
}

/* Register operations */
void i_wl16(Instruction *inst)
{
  GR[inst->i16.operand] = ((unsigned int)GR[inst->i16.operand] & 0xffff0000) | (immediate_i16(inst) & 0xffff);
}

void i_wh16(Instruction *inst)
{
  GR[inst->i16.operand] = ((unsigned int)GR[inst->i16.operand] & 0xffff) | ((immediate_i16(inst) & 0xffff) << 16);
}

void i_clrb(Instruction *inst)
{
  GR[inst->i11.operand] &= ~((unsigned int)0x01 << immediate_i11(inst));
}

void i_setb(Instruction *inst)
{
  GR[inst->i11.operand] |= (unsigned int)0x01 << immediate_i11(inst);
}

void i_clr(Instruction *inst)
{
  GR[inst->o1.operand1] = 0x00000000;
}

void i_set(Instruction *inst)
{
  GR[inst->o1.operand1] = (unsigned int)0xffffffff;
}

void i_revb(Instruction *inst)
{
  /* FIXME: not implement */
  fprintf(stderr, "[Error] %s not implemented yet.\n", "revb");
  exit(EXIT_FAILURE);
}

void i_rev8(Instruction *inst)
{
  unsigned int *dest, src;

  ops_o2_ui11(inst, &dest, &src);

  *dest = htobe32(src);
}

void i_getb(Instruction *inst)
{
  /* FIXME: not implement */
  fprintf(stderr, "[Error] %s not implemented yet.\n", "getb");
  exit(EXIT_FAILURE);
}

void i_get8(Instruction *inst)
{
  unsigned int *dest, src;

  ops_o2_ui11(inst, &dest, &src);

  *dest = ((*dest) >> (src * 4)) & 0xff;
}

void i_lil(Instruction *inst)
{
  GR[inst->i16.operand] = immediate_i16(inst);
}

void i_lih(Instruction *inst)
{
  GR[inst->i16.operand] = (unsigned int)immediate_ui16(inst) << 16;
}

void i_ulil(Instruction *inst)
{
  GR[inst->i16.operand] = immediate_ui16(inst);
}

/* Load, Store */
void i_ld8(Instruction *inst)
{
  unsigned int *dest, src;

  ops_o2_ui11(inst, &dest, &src);

  if(src >= IOSR) {
    errx(EXIT_FAILURE, "ld8: only word access accepted in IO area.");
  }

  *dest = *((unsigned char *)MEMP8(src));
  DEBUGLD("[Load ] Addr: 0x%08x, Data:       0x%02x, PC: 0x%08x\n", src, (unsigned char)*dest, PCR);
  DEBUGLDHW("[L], %08x, %08x, %08x, %08x\n", PCR, SPR, src, (unsigned char)*dest);
}

void i_ld16(Instruction *inst)
{
  unsigned int *dest, src;

  ops_o2_ui11(inst, &dest, &src);

  if(inst->i11.is_immediate) {
    src <<= 1;
  }

  if(src & 0x1) {
    errx(EXIT_FAILURE, "ld16: invalid alignment.");
  }
  else if(src >= IOSR) {
    errx(EXIT_FAILURE, "ld16: only word access accepted in IO area.");
  }

  *dest = *((unsigned short *)MEMP16(src));
  DEBUGLD("[Load ] Addr: 0x%08x, Data:     0x%04x, PC: 0x%08x\n", src, (unsigned short)*dest, PCR);
  DEBUGLDHW("[L], %08x, %08x, %08x, %08x\n", PCR, SPR, src, (unsigned short)*dest);
}

void i_ld32(Instruction *inst)
{
  unsigned int *dest, src;

  ops_o2_ui11(inst, &dest, &src);

  if(inst->i11.is_immediate) {
    src <<= 2;
  }

  if(src & 0x3) {
    errx(EXIT_FAILURE, "ld32: invalid alignment.");
  }
  else if(src >= IOSR) {
    io_load(src);
  }

  *dest = *((unsigned int *)MEMP(src));
  DEBUGLD("[Load ] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", src, *dest, PCR);
  DEBUGLDHW("[L], %08x, %08x, %08x, %08x\n", PCR, SPR, src, *dest);
}

void i_st8(Instruction *inst)
{
  unsigned int *dest, src;

  ops_o2_ui11(inst, &dest, &src);

  *((unsigned char *)MEMP8(src)) = (unsigned char)*dest;
  DEBUGST("[Store] Addr: 0x%08x, Data:       0x%02x, PC: 0x%08x\n", src, (unsigned char)*dest, PCR);
  DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, src, (unsigned char)*dest);

  if(src >= IOSR) {
    errx(EXIT_FAILURE, "st8: only word access accepted in IO area.");
  }
}

void i_st16(Instruction *inst)
{
  unsigned int *dest, src;

  ops_o2_ui11(inst, &dest, &src);

  if(inst->i11.is_immediate) {
    src <<= 1;
  }

  if(src & 0x1) {
    errx(EXIT_FAILURE, "st16: invalid alignment.");
  }

  *((unsigned short *)MEMP16(src)) = (unsigned short)*dest;
  DEBUGST("[Store] Addr: 0x%08x, Data:     0x%04x, PC: 0x%08x\n", src, (unsigned short)*dest, PCR);
  DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, src, (unsigned short)*dest);

  if(src >= IOSR) {
    errx(EXIT_FAILURE, "st16: only word access accepted in IO area.");
  }
}

void i_st32(Instruction *inst)
{
  unsigned int *dest, src;

  ops_o2_ui11(inst, &dest, &src);

  if(inst->i11.is_immediate) {
    src <<= 2;
  }

  if(src & 0x3) {
    errx(EXIT_FAILURE, "st32: invalid alignment.");
  }

  *((unsigned int *)MEMP(src)) = *dest;
  DEBUGST("[Store] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", src, *dest, PCR);
  DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, src, *dest);

  if(src >= IOSR) {
    io_store(src);
  }
}

/* Stack */
void i_push(Instruction *inst)
{
  SPR -= 4;
  *((int *)MEMP(SPR)) = GR[inst->o1.operand1];

  DEBUGST("[Push] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", SPR, GR[inst->o1.operand1], PCR);
  DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, SPR, GR[inst->o1.operand1]);
}

void i_pushpc(Instruction *inst)
{
  SPR -= 4;
  *((int *)MEMP(SPR)) = PCR;
}

void i_pop(Instruction *inst)
{
  GR[inst->o1.operand1] = *((int *)MEMP(SPR));
  SPR += 4;

  DEBUGLD("[Pop] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", SPR - 4, GR[inst->o1.operand1], PCR);
  DEBUGLDHW("[L], %08x, %08x, %08x, %08x\n", PCR, SPR, SPR - 4, GR[inst->o1.operand1]);
}

/* Branch */
void i_bur(Instruction *inst)
{
  if(check_condition(inst)) {
    DEBUGJMP("[Branch] URE: 0x%08x, Cond: %X, PC: 0x%08x\n", PCR + src_jo1_jui16(inst), inst->ji16.condition, PCR);
    next_PCR = PCR + src_jo1_jui16(inst);
  }
}

void i_br(Instruction *inst)
{
  if(check_condition(inst)) {
    DEBUGJMP("[Branch] REL: 0x%08x, Cond: %X, PC: 0x%08x\n", PCR + src_jo1_ji16(inst), inst->ji16.condition, PCR);
    next_PCR = PCR + src_jo1_ji16(inst);
  }
}

void i_b(Instruction *inst)
{
  if(check_condition(inst)) {
    DEBUGJMP("[Branch]  D : 0x%08x, Cond: %X, PCR: 0x%08x\n", src_jo1_jui16(inst), inst->ji16.condition, PCR);
    next_PCR = src_jo1_jui16(inst);
  }

  /* for traceback */
  if(!inst->jo1.is_immediate && inst->jo1.operand1 == GR_RET) {
    traceback_next--;
  }
}

void i_ib(Instruction *inst)
{
  interrupt_exit();
}

void i_srspr(Instruction *inst)
{
  GR[inst->o1.operand1] = SPR;
}

void i_srpdtr(Instruction *inst)
{
  GR[inst->o1.operand1] = PDTR;
}

void i_srpidr(Instruction *inst)
{
  /* FIXME: not implemented */
  GR[inst->o1.operand1] = 0;
}

void i_srcidr(Instruction *inst)
{
  /* FIXME: not implemented */
  GR[inst->o1.operand1] = 0;
}

void i_srmoder(Instruction *inst)
{
  /* FIXME: not implemented */
  GR[inst->o1.operand1] = 0;
}

void i_srieir(Instruction *inst)
{
  GR[inst->o1.operand1] = (PSR & PSR_IM_ENABLE) >> 2;
}

void i_srmmur(Instruction *inst)
{
  GR[inst->o1.operand1] = (PSR & PSR_MMUMOD_MASK);
}

void i_sriosr(Instruction *inst)
{
  GR[inst->o1.operand1] = IOSR;
}

void i_srtidr(Instruction *inst)
{
  GR[inst->o1.operand1] = TIDR;
}

void i_srppsr(Instruction *inst)
{
  GR[inst->o1.operand1] = PPSR;
}

void i_srppcr(Instruction *inst)
{
  GR[inst->o1.operand1] = PPCR;
}

void i_srppdtr(Instruction *inst)
{
  GR[inst->o1.operand1] = PPDTR;
}

void i_srptidr(Instruction *inst)
{
  GR[inst->o1.operand1] = PTIDR;
}

void i_srpsr(Instruction *inst)
{
  GR[inst->o1.operand1] = PSR;
}

void i_srfrcr(Instruction *inst)
{
  FRCR = (unsigned long long)clock();
}

void i_srfrclr(Instruction *inst)
{
  GR[inst->o1.operand1] = (unsigned int)(FRCR & 0xffffffff);
}

void i_srfrchr(Instruction *inst)
{
  GR[inst->o1.operand1] = (unsigned int)(FRCR >> 32);
}

void i_srpflagr(Instruction *inst)
{
  GR[inst->o1.operand1] = PFLAGR.flags;
}

void i_srspw(Instruction *inst)
{
  SPR = GR[inst->o1.operand1];
}

void i_srpdtw(Instruction *inst)
{
  PDTR = GR[inst->o1.operand1];
  DEBUGMMU("[MMU] SRPDTW: 0x%08x\n", PDTR);
}

void i_srieiw(Instruction *inst)
{
  if(src_o1_i11(inst) & 1) {
    PSR |= PSR_IM_ENABLE;
  }
  else {
    PSR &= ~PSR_IM_ENABLE;
  }

  DEBUGINT("[INTERRUPT] SRIEIW: Interrupt %s\n", (PSR & PSR_IM_ENABLE) ? "Enabled" : "Disabled");
}

void i_srmmuw(Instruction *inst)
{
  PSR = (PSR & ~PSR_MMUMOD_MASK) | (src_o1_i11(inst) & PSR_MMUMOD_MASK);
  DEBUGMMU("[MMU] SRMMUW: MMUMOD %d\n", PSR & PSR_MMUMOD_MASK);
}

void i_srppsw(Instruction *inst)
{
  PPSR = (Memory)GR[inst->o1.operand1];
}

void i_srppcw(Instruction *inst)
{
  PPCR = (Memory)GR[inst->o1.operand1];
}

void i_srppdtw(Instruction *inst)
{
  PPDTR = (Memory)GR[inst->o1.operand1];
}

void i_srptidw(Instruction *inst)
{
  PTIDR = (Memory)GR[inst->o1.operand1];
}

void i_sridtw(Instruction *inst)
{
  IDTR = (Memory)GR[inst->o1.operand1];
  DEBUGINT("[INTERRUPT] SRIDTW: idtr <= 0x%08x\n", IDTR);
}

void i_srpsw(Instruction *inst)
{
  PSR = GR[inst->o1.operand1];
  DEBUGMMU("[MMU] SRPSW: MMUMOD %d MMUPS %d\n", PSR_MMUMOD, PSR_MMUPS);
}

void i_srpflagw(Instruction *inst)
{
  PFLAGR.flags = GR[inst->o1.operand1];
}

void i_srspadd(Instruction *inst)
{
  SPR += (int)SIGN_EXT16(inst->c.immediate << 2);
}

void i_nop(Instruction *inst)
{
  return;
}

void i_halt(Instruction *inst)
{
  exit(EXIT_FAILURE);
}

void i_move(Instruction *inst)
{
  GR[inst->o2.operand1] = GR[inst->o2.operand2];
}

void i_movepc(Instruction *inst)
{
  int *dest, src;

  ops_o2_i11(inst, &dest, &src);
  *dest = PCR + (src << 2);

  /* for traceback */
  if(inst->o2.operand1 == GR_RET) {
    traceback[traceback_next++] = PCR + 4;
  }
}

void i_swi(Instruction *inst)
{
  unsigned int num;

  num = immediate_i11(inst);

  if(num < IDT_SWIRQ_START_NUM) {
    /* software irq number should be 64 or above */
    errx(EXIT_FAILURE, "swi: invalid swi (%d)", num);
  }

  interrupt_dispatch_nonmask(num);
}

void i_idts(Instruction *inst)
{
  interrupt_idt_store();
}
