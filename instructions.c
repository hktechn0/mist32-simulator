#include "common.h"

void i_nop(Instruction *inst) {
  return;
}

/* Arithmetic */
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
  gr[inst->o2.operand1] = -gr[inst->o2.operand2];
}

/* Shift, Rotate */
void i_lshl(Instruction *inst) {
  if(inst->i5.is_immediate) {
    (unsigned int)gr[inst->i5.operand] <<= inst->i5.immediate;
  }
  else {
    (unsigned int)gr[inst->o2.operand1] <<= gr[inst->o2.operand2];
  }
}

void i_lshr(Instruction *inst) {
  if(inst->i5.is_immediate) {
    (unsigned int)gr[inst->i5.operand] >>= inst->i5.immediate;
  }
  else {
    (unsigned int)gr[inst->o2.operand1] >>= gr[inst->o2.operand2];
  }
}

void i_ashr(Instruction *inst) {
  if(inst->i5.is_immediate) {
    (signed int)gr[inst->i5.operand] >>= inst->i5.immediate;
  }
  else {
    (signed int)gr[inst->o2.operand1] >>= gr[inst->o2.operand2];
  }
}

void i_ror(Instruction *inst) {
  unsigned int n;
  unsigned int *dest;
  
  if(inst->i11.is_immediate) {
    n = immediate_i11(inst);
    dest = (unsigned int *)&(gr[inst->i11.operand]);
  }
  else {
    n = gr[inst->o2.operand2];
    dest = (unsigned int *)&(gr[inst->o2.operand1]);
  }

  *dest = (*dest << (32 - n)) & (*dest >> n);
}


/* Logic */
void i_and(Instruction *inst) {
  gr[inst->o2.operand1] &= gr[inst->o2.operand2];
}

void i_or(Instruction *inst) {
  gr[inst->o2.operand1] |= gr[inst->o2.operand2];
}

void i_not(Instruction *inst) {
  gr[inst->o2.operand1] = ~gr[inst->o2.operand2];
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
