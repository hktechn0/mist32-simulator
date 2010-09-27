#include "common.h"

unsigned int immediate_i11(Instruction *inst) {
  return (inst->i11.immediate1 << 10) + inst->i11.immediate2;
}

unsigned int immediate_i16(Instruction *inst) {
  return (inst->i16.immediate1 << 10) + inst->i16.immediate2;
}

void ops_o2_i5(Instruction *inst, int **op1, int *op2) {
  if(inst->i5.is_immediate) {
    *op1 = &(gr[inst->i5.operand]);
    *op2 = inst->i5.immediate;
  }
  else {
    *op1 = &(gr[inst->o2.operand1]);
    *op2 = gr[inst->o2.operand2];
  }
}

void ops_o2_i11(Instruction *inst, int **op1, int *op2) {
  if(inst->i11.is_immediate) {
    *op1 = &(gr[inst->i11.operand]);
    *op2 = immediate_i11(inst);
  }
  else {
    *op1 = &(gr[inst->o2.operand1]);
    *op2 = gr[inst->o2.operand2];
  }
}

/* Flags */
void clr_flags(void) {
  *(unsigned int *)(&flags) = 0;
}

void set_flags(int value) {
  flags.zero = !value;
  flags.parity = !(value & 0x00000001);
  flags.sign = (value < 0);
}

void set_overflow(unsigned int d, unsigned int s, unsigned int r) {
  return;
}

void set_carry(unsigned int d, unsigned int s, unsigned int r) {
  return;
}
