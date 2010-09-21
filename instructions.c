#include "common.h"

void i_nop(Instruction *inst) {
  return;
}

void i_add(Instruction *inst) {
  if(inst->i11.is_immediate) {
    gr[inst->i11.operand] += immediate_i11(inst);
  }
  else {
    gr[inst->o2.operand1] += gr[inst->o2.operand2];
  }
}

void i_sub(Instruction *inst) {
  if(inst->i11.is_immediate) {
    gr[inst->i11.operand] -= immediate_i11(inst);
  }
  else {
    gr[inst->o2.operand1] -= gr[inst->o2.operand2];
  }
}

void i_mul(Instruction *inst) {
}

void i_div(Instruction *inst) {
}

void i_sch(Instruction *inst) {
}

void i_and(Instruction *inst) {
  gr[inst->o2.operand1] &= gr[inst->o2.operand2];
}

void i_or(Instruction *inst) {
  gr[inst->o2.operand1] |= gr[inst->o2.operand2];
}

void i_not(Instruction *inst) {
  return;
}

void i_exor(Instruction *inst) {
  gr[inst->o2.operand1] = ((gr[inst->o2.operand1] & gr[inst->o2.operand2]) & 
			   (gr[inst->o2.operand1] | gr[inst->o2.operand2]));
}

void i_nand(Instruction *inst) {
  gr[inst->o2.operand1] = ~(gr[inst->o2.operand1] & gr[inst->o2.operand2]);
}

void i_nor(Instruction *inst) {
  gr[inst->o2.operand1] = ~(gr[inst->o2.operand1] | gr[inst->o2.operand2]);
}

void i_push(Instruction *inst) {
}

void i_pop(Instruction *inst) {
}

void i_halt(Instruction *inst) {
  return;
}
