#include <stdio.h>
#include <stdlib.h>
#include <err.h>

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

bool step_by_step = false;
bool exec_finish = false;

void signal_on_sigint(int signo)
{
  /* EXIT */
  exec_finish = true;

  if(MONITOR) {
    monitor_disconnect();
  }

  fprintf(stderr, "[System] Keyboard Interrupt.\n");
}

int exec(Memory entry_p)
{
  Instruction *inst;
  OpcodeTable opcode_t;
  unsigned long clk = 0;

  unsigned int i;
  char c;

  if(signal(SIGINT, signal_on_sigint) == SIG_ERR) {
    err(EXIT_FAILURE, "signal SIGINT");
  }

  /*
  for(i = 0; i < breakp_next; i++) {
    printf("Break point[%d]: 0x%08x\n", i, breakp[i]);
  }
  */

  /* opcode table init */
  opcode_t = opcode_table_init();

  /* set stack pointer (bottom of memory) */
  SPR = (Memory)STACK_DEFAULT;
  PCR = entry_p;
  PSR = 0;

  printf("Execution Start: entry = 0x%08x\n", PCR);

  do {
    next_PCR = ~0;

    /* break point check */
    for(i = 0; i < breakp_next; i++) {
      if(PCR == breakp[i]) {
	step_by_step = true;
	break;
      }
    }

    /* instruction fetch */
    inst = (Instruction *)MEMP(PCR);

    if(DEBUG || step_by_step) {
      puts("---");
      print_instruction(inst);
    }

    /* decode */
    if(opcode_t[inst->base.opcode] == NULL) {
      errx(EXIT_FAILURE, "invalid opcode. (pc:%08x op:%x)", PCR, inst->base.opcode);
    }

    /* execution */
    (*(opcode_t[inst->base.opcode]))(inst);

    if(step_by_step) {
      print_registers();
      print_stack(SPR);
      dps_info();

      printf("> ");
      c = getchar();

      if(c == 'c') {
	step_by_step = false;
      }
      else if(c == 'q') {
	exec_finish = true;
      }
    }
    else {
      if(DEBUG_REG) { print_registers(); }
      if(DEBUG_STACK) { print_stack(SPR); }
      if(DEBUG_DPS) { dps_info(); }
    }

    if(!(clk & MONITOR_RECV_INTERVAL_MASK)) {
      if((PSR & PSR_IM_ENABLE) && IDT_ISENABLE(IDT_DPS_LS_NUM)) {
	dps_sci_recv();
      }

      if(MONITOR) {
	monitor_method_recv();
      }
    }

    /* next */
    if(next_PCR != ~0) {
      PCR = next_PCR;
    }
    else {
      PCR += 4;
    }

    interrupt_dispatcher();
    clk++;

  } while(!(PCR == 0 && GR[31] == 0) && !exec_finish);
  /* exit if b rret && rret == 0 */

  puts("---- Program Terminated ----");
  print_instruction(inst);
  print_registers();

  free(opcode_t);

  return 0;
}
