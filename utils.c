#include <stdio.h>
#include "common.h"

void print_instruction(Instruction *inst)
{
  printf("PC: 0x%08x Op: 0x%03x(%4d) Insn: 0x%08x SP: 0x%08x\n",
	 PCR, inst->base.opcode, inst->base.opcode, inst->value, SPR);
}

void print_registers(void)
{
  unsigned int i;  
  
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
    if(i >= MEMORY_MAX_ADDR) { break; }
    data = *(unsigned int *)MEMP(i);
    printf("0x%08x: 0x%08x (%11d)\n", i, data, data);
  }
}
