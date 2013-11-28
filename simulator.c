#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>

#include "common.h"
#include "interrupt.h"
#include "monitor.h"

Memory mem;

/* General Register */
int GR[32];

/* System Register */
FLAGS FLAGR;
Memory PCR, next_PCR;
Memory SPR, KSPR, USPR;
unsigned int PSR;
Memory IOSR;
Memory PDTR;
Memory IDTR;
unsigned int TIDR;
unsigned long long FRCR;

Memory traceback[TRACEBACK_MAX];
unsigned int traceback_next = 0;

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

  int memfd;

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

  /* setup system registers */
  PSR = 0;
  PCR = entry_p;
  KSPR = (Memory)STACK_DEFAULT;

  printf("Execution Start: entry = 0x%08x\n", PCR);

  do {
    next_PCR = ~0;
    SPR = (PSR & PSR_CMOD_MASK) ? USPR : KSPR;

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
      abort_sim();
      errx(EXIT_FAILURE, "invalid opcode. (pc:%08x op:%x)", PCR, inst->base.opcode);
    }

    /* execution */
    (*(opcode_t[inst->base.opcode]))(inst);

    /* writeback SP */
    if(PSR & PSR_CMOD_MASK) {
      USPR = SPR;
    }
    else {
      KSPR = SPR;
    }

    if(step_by_step) {
      print_registers();
      print_traceback();
      // print_stack(SPR);
      // dps_info();

      printf("> ");

      while((c = getchar()) == -1);

      if(c == 'c') {
	step_by_step = false;
      }
      else if(c == 'q') {
	exec_finish = true;
      }
      else if(c == 'm') {
	memfd = open("memory.dump", O_WRONLY | O_CREAT, S_IRWXU);
	write(memfd, MEMP(0), 0x1000);
	close(memfd);
      }
    }
    else {
      if(DEBUG_REG) { print_registers(); }
      if(DEBUG_TRACE) { print_traceback(); }
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

  } while(!(PCR == 0 && GR[31] == 0 && DEBUG_EXIT_B0) && !exec_finish);
  /* DEBUG_EXIT_B0: exit if b rret && rret == 0 */

  puts("---- Program Terminated ----");
  print_instruction(inst);
  print_registers();

  free(opcode_t);

  return 0;
}
