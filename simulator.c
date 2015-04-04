#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "common.h"
#include "interrupt.h"
#include "monitor.h"
#include "fetch.h"

#include "instructions.h"
#include "dispatch.h"     /* see opsgen.py */

/* General Register */
int32_t GR[32] __attribute__ ((aligned(64)));

/* System Register */
Memory PCR, next_PCR;
Memory SPR, KSPR, USPR;
FLAGS FLAGR;
uint32_t PSR;
Memory IOSR;
Memory PDTR, KPDTR;
Memory IDTR;
uint32_t TIDR;
uint64_t FRCR;
uint32_t FI0R, FI1R;

bool step_by_step;
bool exec_finish;

Memory prefetch_pc;

Memory traceback[TRACEBACK_MAX];
uint32_t traceback_next;

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
  Instruction insn;
  unsigned long clk = 0;

  uint32_t cmod;

  if(signal(SIGINT, signal_on_sigint) == SIG_ERR) {
    err(EXIT_FAILURE, "signal SIGINT");
  }

  /*
  for(unsigned int i = 0; i < breakp_next; i++) {
    printf("Break point[%d]: 0x%08x\n", i, breakp[i]);
  }
  */

  step_by_step = false;
  exec_finish = false;

  /* initialize internal variable */
  traceback_next = 0;
  memory_is_fault = 0;
  memory_io_writeback = 0;
  instruction_prefetch_flush();

  /* setup system registers */
  PSR = 0;
  cmod = (PSR & PSR_CMOD_MASK);
  PCR = entry_p;
  next_PCR = 0xffffffff;
  KSPR = (Memory)STACK_DEFAULT;

  printf("Execution Start: entry = 0x%08x\n", PCR);

  do {
    /* choose stack */
    if(cmod != (PSR & PSR_CMOD_MASK)) {
      SPR = !cmod ? USPR : KSPR;
    }
    cmod = (PSR & PSR_CMOD_MASK);

#if !NO_DEBUG
    /* break point check */
    for(unsigned int i = 0; i < breakp_next; i++) {
      if(PCR == breakp[i]) {
	step_by_step = true;
	break;
      }
    }
#endif

    /* instruction fetch */
#if CACHE_L1_I_ENABLE
    insn.value = instruction_fetch_cache(PCR);
#else
    insn.value = instruction_fetch(PCR);
#endif

    if(memory_is_fault) {
      /* fault fetch */
      DEBUGINT("[FAULT] Instruction fetch: %08x\n", PCR);
      goto fault;
    }

#if !NO_DEBUG
    if(DEBUG || step_by_step) {
      puts("---");
      print_instruction(insn);
    }
#endif

    /* execution */
    insn_dispatch(insn);

  fault:
    if(memory_is_fault) {
      /* faulting memory access */
      interrupt_dispatch_nonmask(memory_is_fault);
      next_PCR = PCR;

      memory_io_writeback = 0;
      memory_is_fault = 0;
    }
    else if(memory_io_writeback) {
      /* sync io */
      io_store(memory_io_writeback);
      memory_io_writeback = 0;
    }

    /* writeback SP */
    if(cmod) {
      USPR = SPR;
    }
    else {
      KSPR = SPR;
    }

#if !NO_DEBUG
    if(step_by_step) {
      step_by_step_pause();
    }
    else {
      if(DEBUG_REG) { print_registers(); }
      if(DEBUG_TRACE) { print_traceback(); }
      if(DEBUG_STACK) { print_stack(SPR); }
      if(DEBUG_DPS) { dps_info(); }
    }
#endif

    if(!(clk & MONITOR_RECV_INTERVAL_MASK)) {
      if((PSR & PSR_IM_ENABLE) && IDT_ISENABLE(IDT_DPS_LS_NUM)) {
	dps_sci_recv();
      }

      if(MONITOR) {
	monitor_method_recv();
      }
    }

    /* next */
    if(next_PCR != 0xffffffff) {
      /* alignment check */
      if(next_PCR & 0x3) {
	abort_sim();
	errx(EXIT_FAILURE, "invalid branch addres. %08x", next_PCR);
      }

      PCR = next_PCR;
      next_PCR = 0xffffffff;
    }
    else {
      PCR += 4;
    }

    interrupt_dispatcher();
    clk++;

  } while(!(PCR == 0 && GR[31] == 0 && DEBUG_EXIT_B0) && !exec_finish);
  /* DEBUG_EXIT_B0: exit if b rret && rret == 0 */

  puts("---- Program Terminated ----");
  print_instruction(insn);
  print_registers();

  return 0;
}
