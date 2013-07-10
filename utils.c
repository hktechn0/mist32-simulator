#include <stdio.h>
#include <limits.h>
#include "common.h"

/* fetch immediate for i11 format */
unsigned int immediate_ui11(Instruction *inst)
{
  return (inst->i11.immediate1 << 5) + inst->i11.immediate2;
}

int immediate_i11(Instruction *inst)
{
  int imm;
  
  /* sign extend */
  imm = ((int)immediate_ui11(inst) << 21) >> 21;
  
  return imm;
}

/* fetch immediate for i16 format */
unsigned int immediate_ui16(Instruction *inst)
{
  return (inst->i16.immediate1 << 5) + inst->i16.immediate2;
}

int immediate_i16(Instruction *inst)
{
  int imm;

  /* sign extend */
  imm = ((int)immediate_ui16(inst) << 16) >> 16;
  
  return imm;
}

/* fetch o2 or i11 format operand
  inst: Instruction struct,
  op1:  store operand1 pointer (writable, directly to register),
  op2:  store operand2 (read only) */
void ops_o2_i11(Instruction *inst, int **op1, int *op2)
{
  if(inst->i11.is_immediate) {
    *op1 = &(GR[inst->i11.operand]);
    *op2 = immediate_i11(inst);
  }
  else {
    *op1 = &(GR[inst->o2.operand1]);
    *op2 = GR[inst->o2.operand2];
  }
}

void ops_o2_ui11(Instruction *inst, unsigned int **op1, unsigned int *op2)
{
  if(inst->i11.is_immediate) {
    *op1 = (unsigned int *)&(GR[inst->i11.operand]);
    *op2 = immediate_ui11(inst);
  }
  else {
    *op1 = (unsigned int *)&(GR[inst->o2.operand1]);
    *op2 = (unsigned int)GR[inst->o2.operand2];
  }
}

/* return source operand value (I11, O2 format) */
int src_o2_i11(Instruction *inst)
{
  if(inst->i11.is_immediate) {
    return immediate_i11(inst);
  }
  else {
    return GR[inst->o2.operand2];
  }
}

/* return source operand value (I11, O1 format) */
int src_o1_i11(Instruction *inst)
{
  if(inst->i11.is_immediate) {
    return immediate_i11(inst);
  }
  else {
    return GR[inst->o1.operand1];
  }
}

/* return source operand value (JI16, JO1 format) */
int src_jo1_ji16(Instruction *inst)
{
  if(inst->ji16.is_immediate) {
    return ((int)inst->ji16.immediate << 16) >> 14;
  }
  else {
    return GR[inst->jo1.operand1];
  }
}

/* return source operand value (JI16, JO1 format) */
unsigned int src_jo1_jui16(Instruction *inst)
{
  if(inst->ji16.is_immediate) {
    /* no sign extend */
    return inst->ji16.immediate << 2;
  }
  else {
    return GR[inst->jo1.operand1];
  }
}

/* Check condition code and flags */
/* return: true, false */
int check_condition(Instruction *inst)
{
  switch(inst->ji16.condition) {
  case 0:
    return 1;
    break;
  case 1:
    if(FLAGR.zero) { return 1; }
    else { return 0; }
    break;
  case 2:
    if(!FLAGR.zero) { return 1; }
    else { return 0; }
    break;
  case 3:
    if(FLAGR.sign) { return 1; }
    else { return 0; }
    break;
  case 4:
    if(!FLAGR.sign) { return 1; }
    else { return 0; }
    break;
  case 5:
    if(FLAGR.parity) { return 1; }
    else { return 0; }
    break;
  case 6:
    if(!FLAGR.parity) { return 1; }
    else { return 0; }
    break;
  case 7:
    if(FLAGR.overflow) { return 1; }
    else { return 0; }
    break;
  case 8:
    if(FLAGR.carry) { return 1; }
    else { return 0; }
    break;
  case 9:
    if(!FLAGR.carry) { return 1; }
    else { return 0; }
    break;
  case 0xA:
    if(FLAGR.carry && !FLAGR.zero) { return 1; }
    else { return 0; }
    break;
  case 0xB:
    if(!FLAGR.carry || FLAGR.zero) { return 1; }
    else { return 0; }
    break;
  case 0xC:
    if(FLAGR.sign == FLAGR.overflow) { return 1; }
    else { return 0; }
    break;
  case 0xD:
    if(FLAGR.sign != FLAGR.overflow) { return 1; }
    else { return 0; }
    break;
  case 0xE:
    if(!((FLAGR.sign ^ FLAGR.overflow) || FLAGR.zero)) { return 1; }
    else { return 0; }
    break;
  case 0xF:
    if((FLAGR.sign ^ FLAGR.overflow) || FLAGR.zero) { return 1; }
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
  FLAGR.sign = 0;
  FLAGR.overflow = 0;
  FLAGR.carry = 0;
  FLAGR.parity = 0;
  FLAGR.zero = 0;
}

void set_flags(int value)
{
  FLAGR.zero = !value;
  FLAGR.parity = !(value & 0x00000001);
  FLAGR.sign = (value < 0);
}

void set_flags_add(unsigned int result, unsigned int dest, unsigned int src)
{
  clr_flags();
  FLAGR.overflow = msb(~(dest ^ src) & (dest ^ result));
  FLAGR.carry = msb((dest & src) | (~result & (dest | src)));
  set_flags(result);
  /*printf("r:%d d:%d s:%d c:%d o:%d\n", (int)result, (int)dest, (int)src, FLAGR.carry, FLAGR.overflow);*/
}

void set_flags_sub(unsigned int result, unsigned int dest, unsigned int src)
{
  set_flags_add(result, dest, (unsigned int)(-((int)src)));

  if(src == 0) {
    FLAGR.carry = 1;
  }
}

void print_registers(void)
{
  unsigned int i;  
  
  printf("PC: 0x%08x SP: 0x%08x\n", PCR, SPR);
  printf("ZF: %d, PF: %d, CF: %d, OF: %d, SF %d\n",
	 FLAGR.zero, FLAGR.parity, FLAGR.carry, FLAGR.overflow, FLAGR.sign);
  for(i = 0; i < 32; i++) {
    printf("R%2d: 0x%08x (%11d) ", i, GR[i], GR[i]);
    if(!((i + 1) % 2)) { printf("\n"); }
  }
}

void print_stack(Memory sp)
{
  unsigned int i, data;

  printf("---- Stack ----\n");
  for(i = sp; i - sp < 40; i += 4) {
    if(i == 0xffffffff) { break; }
    data = *(unsigned int *)MEMP(i);
    printf("0x%08x: 0x%08x (%11d)\n", i, data, data);
  }
}
