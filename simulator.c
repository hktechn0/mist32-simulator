#include <stdio.h>
#include <stdlib.h>
#include "common.h"

int gr[32];
struct FLAGS flags;

Memory mem;
Memory sp;
Memory pc;
Memory next_pc;

void print_registers(void)
{
  unsigned int i;  
  
  printf("PC: 0x%08x SP: 0x%08x\n", pc, sp);
  printf("ZF: %d, PF: %d, CF: %d, OF: %d, SF %d\n",
	 flags.zero, flags.parity, flags.carry, flags.overflow, flags.sign);
  for(i = 0; i < 32; i++) {
    printf("R%2d: 0x%08x (%11d) ", i, gr[i], gr[i]);
    if(!((i + 1) % 2)) { printf("\n"); }
  }
}

int exec(Memory entry_p)
{
  Instruction *inst;
  OpcodeTable opcode_t;
  
  /* opcode table init */
  opcode_t = (OpcodeTable)opcode_table_init();
  
  if(DEBUG) {
    printf("Execution Start: entry = 0x%08x\n", pc);
    print_registers();
  }
  
  pc = entry_p;
  
  do {
    next_pc = ~0;

    /* instruction fetch */
    inst = (Instruction *)MEMP(pc);
    
    if(DEBUG) {
      puts("---");
      printf("Op: 0x%03x(%4d) Insn: 0x%08x\n",
	     inst->base.opcode, inst->base.opcode, inst->value);
    }
    
    /* execute operation */
    (*(opcode_t[inst->base.opcode]))(inst);
    
    if(DEBUG) {
      print_registers();
      getchar();
    }

    if(next_pc != ~0) {
      pc = next_pc;
    }
    else {
      pc += 4;
    }
  } while(pc != 0);

  puts("Program Terminated");
  free(opcode_t);
  
  return 0;
}
