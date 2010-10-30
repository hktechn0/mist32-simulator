#include "common.h"

/* fetch immediate for i11 format */
unsigned int immediate_i11(Instruction *inst)
{
  unsigned int imm;
  
  imm = (inst->i11.immediate1 << 5) + inst->i11.immediate2;
  
  /* check immediate sign flag */
  if(inst->i11.immediate1 & 0x20) {
    imm |= 0xfffff800;
  }
  
  return imm;
}

/* fetch immediate for i16 format */
unsigned int immediate_i16(Instruction *inst)
{
  unsigned int imm;

  imm = (inst->i16.immediate1 << 5) + inst->i16.immediate2;

  /* check immediate sign flag */
  if(inst->i16.immediate1 & 0x400) {
    imm |= 0xffff0000;
  }
  
  return imm;
}

/* fetch o2 or i5 format operand
  inst: Instruction struct,
  op1:  store operand1 pointer (writable, directly to register),
  op2:  store operand2 (read only) */
void ops_o2_i5(Instruction *inst, int **op1, int *op2)
{
  if(inst->i5.is_immediate) {
    *op1 = &(gr[inst->i5.operand]);
    *op2 = inst->i5.immediate;
  }
  else {
    *op1 = &(gr[inst->o2.operand1]);
    *op2 = gr[inst->o2.operand2];
  }
}

/* fetch o2 or i11 format operand
  inst: Instruction struct,
  op1:  store operand1 pointer (writable, directly to register),
  op2:  store operand2 (read only) */
void ops_o2_i11(Instruction *inst, int **op1, int *op2)
{
  if(inst->i11.is_immediate) {
    *op1 = &(gr[inst->i11.operand]);
    *op2 = immediate_i11(inst);
  }
  else {
    *op1 = &(gr[inst->o2.operand1]);
    *op2 = gr[inst->o2.operand2];
  }
}

/* return source operand value (I11, O2 format) */
int src_o2_i11(Instruction *inst)
{
  if(inst->i11.is_immediate) {
    return immediate_i11(inst);
  }
  else {
    return gr[inst->o2.operand2];
  }
}

/* return source operand value (I11, O2 format) */
int src_o1_i11(Instruction *inst)
{
  if(inst->i11.is_immediate) {
    return immediate_i11(inst);
  }
  else {
    return gr[inst->o1.operand1];
  }
}

/* Check condition code and flags */
/* return: true, false */
int check_condition(Instruction *inst)
{
  switch(inst->ji.condition) {
  case 0:
    return 1;
    break;
  case 1:
    if(flags.zero) { return 1; }
    else { return 0; }
    break;
  case 2:
    if(!flags.zero) { return 1; }
    else { return 0; }
    break;
  case 3:
    if(flags.sign) { return 1; }
    else { return 0; }
    break;
  case 4:
    if(!flags.sign) { return 1; }
    else { return 0; }
    break;
  case 5:
    if(flags.parity) { return 1; }
    else { return 0; }
    break;
  case 6:
    if(!flags.parity) { return 1; }
    else { return 0; }
    break;
  case 7:
    if(flags.overflow) { return 1; }
    else { return 0; }
    break;
  case 8:
    if(flags.carry) { return 1; }
    else { return 0; }
    break;
  case 9:
    if(!flags.carry) { return 1; }
    else { return 0; }
    break;
  case 0xA:
    if(flags.carry && !flags.zero) { return 1; }
    else { return 0; }
    break;
  case 0xB:
    if(!flags.carry || flags.zero) { return 1; }
    else { return 0; }
    break;
  case 0xC:
    if(flags.sign == flags.overflow) { return 1; }
    else { return 0; }
    break;
  case 0xD:
    if(flags.sign != flags.overflow) { return 1; }
    else { return 0; }
    break;
  case 0xE:
    if(!flags.zero && (flags.sign == flags.overflow)) { return 1; }
    else { return 0; }
    break;
  case 0xF:
    if(flags.zero && (!flags.sign == flags.overflow)) { return 1; }
    else { return 0; }
    break;
  default:
    break;
  }
  
  return 0;  
}

/* Flags */
void clr_flags(void)
{
  *(unsigned int *)(&flags) = 0;
}

void set_flags(int value)
{
  flags.zero = !value;
  flags.parity = !(value & 0x00000001);
  flags.sign = (value < 0);
}
