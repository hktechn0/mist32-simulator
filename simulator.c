#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "interrupt.h"
#include "monitor.h"

Memory mem;

/* General Register */
int GR[32];

/* System Register */
struct FLAGS FLAGR;
Memory PCR, next_PCR;
Memory SPR;
unsigned int PSR;
Memory IDTR;
Memory IOSR;
unsigned long long FRCR;

int exec(Memory entry_p)
{
  Instruction *inst;
  OpcodeTable opcode_t;

  /* opcode table init */
  opcode_t = opcode_table_init();

  /* set stack pointer (bottom of memory) */
  SPR = (Memory)STACK_DEFAULT;
  PCR = entry_p;
  PSR = 0;

  if(DEBUG) {
    printf("Execution Start: entry = 0x%08x\n", PCR);
    print_registers();
  }

  do {
    next_PCR = ~0;

    /* instruction fetch */
    inst = (Instruction *)MEMP(PCR);

    if(DEBUG) {
      puts("---");
      printf("Op: 0x%03x(%4d) Insn: 0x%08x\n",
	     inst->base.opcode, inst->base.opcode, inst->value);
    }

    /* execute operation */
    (*(opcode_t[inst->base.opcode]))(inst);

    if(DEBUG_REG) { print_registers(); }
    if(DEBUG_STACK) { print_stack(SPR); }
    if(DEBUG_DPS) { dps_info(); }
    if(DEBUG_I) { getchar(); }

    monitor_method_recv();

    if(next_PCR != ~0) {
      PCR = next_PCR;
    }
    else {
      PCR += 4;
    }

    interrupt_dispatcher();

  } while(!(PCR == 0 && GR[31] == 0));
  /* exit if b rret && rret == 0 */

  puts("Program Terminated");
  print_registers();

  free(opcode_t);

  return 0;
}
