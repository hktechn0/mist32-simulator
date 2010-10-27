#include "common.h"
#define msb(word) (!!((word) & 0x80000000))

void i_nop(Instruction *inst) {
  return;
}

/* Arithmetic */
void i_add(Instruction *inst) {
  int *dest;
  int src, result;
  
  ops_o2_i11(inst, &dest, &src);
  
  result = (*dest) + src;
  
  clr_flags();
  flags.overflow = msb(~(*dest) & ~src & result);
  flags.carry = msb((*dest & src) | (~result & (*dest | src)));
  set_flags(result);
  
  *dest = result;
}

void i_sub(Instruction *inst) {
  int *dest;
  int src, result;
  
  ops_o2_i11(inst, &dest, &src);
  
  result = (*dest) - src;
  
  clr_flags();
  flags.overflow = msb(*dest & ~src & ~result);
  flags.carry = ((unsigned int)*dest < (unsigned int)src);
  set_flags(result);
  
  *dest = result;
}

void i_mul(Instruction *inst) {
}

void i_div(Instruction *inst) {
}

void i_sch(Instruction *inst) {
  gr[inst->o2.operand1] = -gr[inst->o2.operand2];
  set_flags(gr[inst->o2.operand1]);
}

/* Shift, Rotate */
void i_lshl(Instruction *inst) {
  unsigned int *dest;
  int n;
  
  ops_o2_i5(inst, (int **)&dest, &n);
  
  clr_flags();
  flags.carry = (*dest) >> (32 - n);
  *dest = (*dest) << n;
  set_flags(*dest);
}

void i_lshr(Instruction *inst) {
  unsigned int *dest;
  int n;
  
  ops_o2_i5(inst, (int **)&dest, &n);
  
  clr_flags();
  flags.carry = (*dest >> (n - 1)) & 0x00000001;
  *dest = (*dest) >> n;
  set_flags(*dest);
}

void i_ashr(Instruction *inst) {
  int *dest;
  int n;
  
  ops_o2_i5(inst, (int **)&dest, &n);
  
  clr_flags();
  flags.carry = ((*dest) >> (n - 1)) & 0x00000001;
  *dest = (*dest) >> n;
  set_flags(*dest);
}

void i_rol(Instruction *inst) {
  unsigned int *dest;
  int n;
  
  ops_o2_i5(inst, (int **)&dest, &n);
  
  *dest = ((*dest) << n) | ((*dest) >> (32 - n));
  
  clr_flags();
  set_flags(*dest);
  flags.carry = !!(*dest & 0x00000001);
}

void i_ror(Instruction *inst) {
  unsigned int *dest;
  int n;
  
  ops_o2_i5(inst, (int **)&dest, &n);
  
  *dest = ((*dest) << (32 - n));
  
  clr_flags();
  set_flags(*dest);
  flags.carry = !!(*dest & 0x80000000);
}

/* Logic */
void i_and(Instruction *inst) {
  gr[inst->o2.operand1] &= gr[inst->o2.operand2];
  clr_flags();
  set_flags(gr[inst->o2.operand1]);
}

void i_or(Instruction *inst) {
  gr[inst->o2.operand1] |= gr[inst->o2.operand2];
  clr_flags();
  set_flags(gr[inst->o2.operand1]);
}

void i_not(Instruction *inst) {
  gr[inst->o2.operand1] = ~gr[inst->o2.operand2];
}

void i_exor(Instruction *inst) {
  gr[inst->o2.operand1] ^= gr[inst->o2.operand2];
  clr_flags();
  set_flags(gr[inst->o2.operand1]);
}

void i_nand(Instruction *inst) {
  gr[inst->o2.operand1] = ~(gr[inst->o2.operand1] & gr[inst->o2.operand2]);
  clr_flags();
  set_flags(gr[inst->o2.operand1]);
}

void i_nor(Instruction *inst) {
  gr[inst->o2.operand1] = ~(gr[inst->o2.operand1] | gr[inst->o2.operand2]);
  clr_flags();
  set_flags(gr[inst->o2.operand1]);
}

/* Register operations */
void i_wb1(Instruction *inst) {
  gr[inst->i16.operand] = ((unsigned int)gr[inst->i16.operand] & 0xffff0000) | (immediate_i16(inst) & 0xffff);
}

void i_wb2(Instruction *inst) {
  gr[inst->i16.operand] = ((unsigned int)gr[inst->i16.operand] & 0xffff) | ((immediate_i16(inst) & 0xffff) << 16);
}

void i_clb(Instruction *inst) {
  gr[inst->i5.operand] &= ~((unsigned int)0x01 << inst->i5.immediate);
}

void i_stb(Instruction *inst) {
  gr[inst->i5.operand] |= (unsigned int)0x01 << inst->i5.immediate;
}

void i_clw(Instruction *inst) {
  gr[inst->o1.operand1] = 0x00000000;
}

void i_stw(Instruction *inst) {
  gr[inst->o1.operand1] = (unsigned int)0xffffffff;
}

/* Load, Store */
void i_load(Instruction *inst) {
  if(inst->i11.is_immediate) {
    gr[inst->i11.operand] = *((unsigned int *)(mem.byte + immediate_i11(inst)));
  }
  else {
    gr[inst->o2.operand1] = *((unsigned int *)(mem.byte + gr[inst->o2.operand2]));
  }
}

void i_store(Instruction *inst) {
  if(inst->i11.is_immediate) {
    *((unsigned int *)(mem.byte + immediate_i11(inst))) = gr[inst->i11.operand];
  }
  else {
    *((unsigned int *)(mem.byte + gr[inst->o2.operand2])) = gr[inst->o2.operand1];
  }
}

/* Stack */
void i_push(Instruction *inst) {
  *(--sp.word) = gr[inst->o1.operand1];
}

void i_pop(Instruction *inst) {
  gr[inst->o1.operand1] = *(sp.word++);
}

void i_mov(Instruction *inst) {
  gr[inst->o2.operand1] = gr[inst->o2.operand2];
}

void i_halt(Instruction *inst) {
  return;
}
