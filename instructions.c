#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <err.h>
#include "common.h"
#include "instructions.h"

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

void i_sub(Instruction *inst)
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

  *dest = *dest / src;
  
  clr_flags();
  /* FIXME: flags unavailable */
}

void i_umod(Instruction *inst)
{
  unsigned int *dest;
  unsigned int src;

  ops_o2_ui11(inst, &dest, &src);

  *dest = *dest % src;
  
  clr_flags();
  /* FIXME: flags unavailable */
}

void i_cmp(Instruction *inst)
{
  int dest;
  
  dest = gr[inst->o2.operand1];
  i_sub(inst);
  gr[inst->o2.operand1] = dest;
}

void i_div(Instruction *inst)
{
  int *dest;
  int src;

  ops_o2_i11(inst, &dest, &src);

  *dest = *dest / src;
  
  clr_flags();
  /* FIXME: flags unavailable */
}

void i_mod(Instruction *inst)
{
  int *dest;
  int src;

  ops_o2_i11(inst, &dest, &src);

  *dest = *dest % src;
  
  clr_flags();
  /* FIXME: flags unavailable */
}

void i_neg(Instruction *inst)
{
  gr[inst->o2.operand1] = -gr[inst->o2.operand2];
  set_flags(gr[inst->o2.operand1]);
  if(gr[inst->o2.operand1] == 0x80000000) { flags.overflow = 1; }
}

void i_addc(Instruction *inst)
{
  int *dest;
  int src, result;
  
  ops_o2_i11(inst, &dest, &src);
  
  result = (*dest) + src;
  set_flags_add(result, *dest, src);
  *dest = flags.carry;
}

void i_inc(Instruction *inst)
{
  int *dest;
  int src, result;
  
  dest = &gr[inst->o2.operand1];
  src = gr[inst->o2.operand2];
  
  result = src + 1;
  set_flags_add(result, src, 1);
  *dest = result;
}

void i_dec(Instruction *inst)
{
  int *dest;
  int src, result;
  
  dest = &gr[inst->o2.operand1];
  src = gr[inst->o2.operand2];
  
  result = src - 1;
  set_flags_sub(result, src, 1);
  *dest = result;
}

void i_sext8(Instruction *inst)
{
  unsigned int *dest, src;
  
  dest = (unsigned int *)&gr[inst->o2.operand1];
  src = (unsigned int)gr[inst->o2.operand2];

  if(src & 0x80) {
    *dest = src | 0xffffff00;
  }
  else {
    *dest = src & 0xff;
  }
}

void i_sext16(Instruction *inst)
{
  unsigned int *dest, src;
  
  dest = (unsigned int *)&gr[inst->o2.operand1];
  src = (unsigned int)gr[inst->o2.operand2];

  if(src & 0x8000) {
    *dest = src | 0xffff0000;
  }
  else {
    *dest = src & 0xffff;
  }
}

/* Shift, Rotate */
void i_shl(Instruction *inst)
{
  unsigned int *dest, n;
  
  ops_o2_ui11(inst, &dest, &n);
  
  clr_flags();
  flags.carry = (*dest) >> (32 - n);
  *dest = (*dest) << n;
  set_flags(*dest);
}

void i_shr(Instruction *inst)
{
  unsigned int *dest, n;
  
  ops_o2_ui11(inst, &dest, &n);
  
  clr_flags();
  flags.carry = (*dest >> (n - 1)) & 0x00000001;
  *dest = (*dest) >> n;
  set_flags(*dest);
}

void i_sar(Instruction *inst)
{
  int *dest;
  unsigned int n;

  ops_o2_i11(inst, &dest, (int *)&n);

  clr_flags();
  flags.carry = ((*dest) >> (n - 1)) & 0x00000001;
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
  flags.carry = !!(*dest & 0x00000001);
}

void i_ror(Instruction *inst)
{
  unsigned int *dest, n;
  
  ops_o2_ui11(inst, &dest, &n);
  
  *dest = ((*dest) << (32 - n));
  
  clr_flags();
  set_flags(*dest);
  flags.carry = !!(*dest & 0x80000000);
}

/* Logic */
void i_and(Instruction *inst)
{
  gr[inst->o2.operand1] &= gr[inst->o2.operand2];
  clr_flags();
  set_flags(gr[inst->o2.operand1]);
}

void i_or(Instruction *inst)
{
  gr[inst->o2.operand1] |= gr[inst->o2.operand2];
  clr_flags();
  set_flags(gr[inst->o2.operand1]);
}

void i_not(Instruction *inst)
{
  gr[inst->o2.operand1] = ~gr[inst->o2.operand2];
}

void i_xor(Instruction *inst)
{
  gr[inst->o2.operand1] ^= gr[inst->o2.operand2];
  clr_flags();
  set_flags(gr[inst->o2.operand1]);
}

void i_nand(Instruction *inst)
{
  i_and(inst);
  NOT(gr[inst->o2.operand1]);

  clr_flags();
  set_flags(gr[inst->o2.operand1]);
}

void i_nor(Instruction *inst)
{
  i_or(inst);
  NOT(gr[inst->o2.operand1]);

  clr_flags();
  set_flags(gr[inst->o2.operand1]);
}

void i_xnor(Instruction *inst)
{
  i_xor(inst);
  NOT(gr[inst->o2.operand1]);

  clr_flags();
  set_flags(gr[inst->o2.operand1]);
}

void i_test(Instruction *inst)
{
  int dest;
  
  dest = gr[inst->o2.operand1];
  i_and(inst);
  gr[inst->o2.operand1] = dest;
}

/* Register operations */
void i_wl16(Instruction *inst)
{
  gr[inst->i16.operand] = ((unsigned int)gr[inst->i16.operand] & 0xffff0000) | (immediate_i16(inst) & 0xffff);
}

void i_wh16(Instruction *inst)
{
  gr[inst->i16.operand] = ((unsigned int)gr[inst->i16.operand] & 0xffff) | ((immediate_i16(inst) & 0xffff) << 16);
}

void i_clrb(Instruction *inst)
{
  gr[inst->i11.operand] &= ~((unsigned int)0x01 << immediate_i11(inst));
}

void i_setb(Instruction *inst)
{
  gr[inst->i11.operand] |= (unsigned int)0x01 << immediate_i11(inst);
}

void i_clr(Instruction *inst)
{
  gr[inst->o1.operand1] = 0x00000000;
}

void i_set(Instruction *inst)
{
  gr[inst->o1.operand1] = (unsigned int)0xffffffff;
}

void i_revb(Instruction *inst)
{
  /* FIXME: not implement */
  fprintf(stderr, "[Error] %s not implemented yet.\n", "revb");
  exit(EXIT_FAILURE);
}

void i_rev8(Instruction *inst)
{
  /* FIXME: not implement */
  fprintf(stderr, "[Error] %s not implemented yet.\n", "rev8");
  exit(EXIT_FAILURE);
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
  gr[inst->i16.operand] = immediate_i16(inst);
}

void i_lih(Instruction *inst)
{
  gr[inst->i16.operand] = (unsigned int)immediate_ui16(inst) << 16;
}

void i_ulil(Instruction *inst)
{
  gr[inst->i16.operand] = immediate_ui16(inst);
}

/* Load, Store */
void i_ld8(Instruction *inst)
{
  unsigned int *dest, src;

  ops_o2_ui11(inst, &dest, &src);

  if(src >= iosr) {
    errx(EXIT_FAILURE, "ld8: only word access accepted in IO area.");
  }

  *dest = *((unsigned char *)MEMP8(src));
  DEBUGLD("[Load ] Addr: 0x%08x, Data:       0x%02x\n", src, (unsigned char)*dest);
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
  else if(src >= iosr) {
    errx(EXIT_FAILURE, "ld16: only word access accepted in IO area.");
  }

  *dest = *((unsigned short *)MEMP16(src));
  DEBUGLD("[Load ] Addr: 0x%08x, Data:     0x%04x\n", src, (unsigned short)*dest);
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
  else if(src >= iosr) {
    io_load(src);
  }

  *dest = *((unsigned int *)MEMP(src));
  DEBUGLD("[Load ] Addr: 0x%08x, Data: 0x%08x\n", src, *dest);
}

void i_st8(Instruction *inst)
{
  unsigned int *dest, src;

  ops_o2_ui11(inst, &dest, &src);

  *((unsigned char *)MEMP8(src)) = (unsigned char)*dest;
  DEBUGST("[Store] Addr: 0x%08x, Data:       0x%02x\n", src, (unsigned char)*dest);

  if(src >= iosr) {
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
  DEBUGST("[Store] Addr: 0x%08x, Data:     0x%04x\n", src, (unsigned short)*dest);

  if(src >= iosr) {
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
  DEBUGST("[Store] Addr: 0x%08x, Data: 0x%08x\n", src, *dest);

  if(src >= iosr) {
    io_store(src);
  }
}

/* Stack */
void i_push(Instruction *inst)
{
  sp -= 4;
  *((int *)MEMP(sp)) = gr[inst->o1.operand1];
}

void i_pushpc(Instruction *inst)
{
  sp -= 4;
  *((int *)MEMP(sp)) = pc;
}

void i_pop(Instruction *inst)
{
  gr[inst->o1.operand1] = *((int *)MEMP(sp));
  sp += 4;
}

/* Branch */
void i_bur(Instruction *inst)
{
  if(check_condition(inst)) {
    next_pc = pc + src_jo1_jui16(inst);
  }
}

void i_br(Instruction *inst)
{
  if(check_condition(inst)) {
    DEBUGJMP("[Branch]   R: 0x%08x, Cond: %X, PC: 0x%08x\n", pc + src_jo1_ji16(inst), inst->ji16.condition, pc);
    next_pc = pc + src_jo1_ji16(inst);
  }
}

void i_b(Instruction *inst)
{
  if(check_condition(inst)) {
    DEBUGJMP("[Branch]   D: 0x%08x, Cond: %X, PC: 0x%08x\n", src_jo1_jui16(inst), inst->ji16.condition, pc);
    next_pc = src_jo1_jui16(inst);
  }
}

void i_ib(Instruction *inst)
{
  /* FIXME: not imeplement */
  fprintf(stderr, "[Error] %s not implemented yet.\n", "ib");
  exit(EXIT_FAILURE);
}

void i_srspr(Instruction *inst)
{
  gr[inst->o1.operand1] = sp;
}

void i_srieir(Instruction *inst)
{
  gr[inst->o1.operand1] = (sr1 & 0x4) >> 2;
}

void i_sriosr(Instruction *inst)
{
  gr[inst->o1.operand1] = iosr;
}

void i_sridtr(Instruction *inst)
{
  gr[inst->o1.operand1] = (int)idtr;
  printf("[System] SRIDTR: idtr => 0x%08x\n", idtr);
}

void i_srfrcr(Instruction *inst)
{
  frcr = (unsigned long long)clock();
}

void i_srfrclr(Instruction *inst)
{
  gr[inst->o1.operand1] = (unsigned int)(frcr & 0xffffffff);
}

void i_srfrchr(Instruction *inst)
{
  gr[inst->o1.operand1] = (unsigned int)(frcr >> 32);
}

void i_srspw(Instruction *inst)
{
  sp = gr[inst->o1.operand1];
}

void i_srieiw(Instruction *inst)
{
  sr1 |= ((gr[inst->o1.operand1] & 1) << 2);
  printf("[System] SRIEIW: Interrupt %s\n", ((sr1 & 0x4) >> 2) ? "Enabled" : "Disabled");
}

void i_sridtw(Instruction *inst)
{
  idtr = (Memory)gr[inst->o1.operand1];
  printf("[System] SRIDTW: idtr <= 0x%08x\n", idtr);
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
  gr[inst->o2.operand1] = gr[inst->o2.operand2];
}

void i_movepc(Instruction *inst)
{
  int *dest, src;

  ops_o2_i11(inst, &dest, &src);

  *dest = pc + (src << 2);
}
