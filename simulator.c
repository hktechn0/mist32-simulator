#include <stdio.h>
#include <stdlib.h>
#include "common.h"

int gr[32];
struct FLAGS flags;

Memory mem;
Memory sp;
Memory pc;
Memory next_pc;

int exec(Memory entry_p)
{
  Instruction *inst;
  OpcodeTable opcode_t;
  
  /* opcode table init */
  opcode_t = (OpcodeTable)opcode_table_init();
  
  /* set stack pointer (bottom of memory) */
  sp = (Memory)STACK_DEFAULT;

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
      print_stack(sp);
      getchar();
    }

    if(next_pc != ~0) {
      pc = next_pc;
    }
    else {
      pc += 4;
    }
  } while(!(pc == 0 && gr[31] == 0));
  /* exit if b rret && rret == 0 */

  puts("Program Terminated");
  print_registers();

  free(opcode_t);
  
  return 0;
}
