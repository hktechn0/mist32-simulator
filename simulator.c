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

/* Opcode Table */
pOpcodeFunc opcode_t[OPCODE_MAX] __attribute__ ((aligned(64)));

/* General Register */
int32_t GR[32] __attribute__ ((aligned(64)));

/* System Register */
FLAGS FLAGR;
Memory PCR, next_PCR;
Memory SPR, KSPR, USPR;
uint32_t PSR;
Memory IOSR;
Memory PDTR;
Memory IDTR;
uint32_t TIDR;
uint64_t FRCR;
uint32_t FI0R, FI1R;

bool step_by_step;
bool exec_finish;

Memory prefetch_pc;
uint32_t prefetch_insn[PREFETCH_N] __attribute__ ((aligned(64)));

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

void signal_on_sigsegv(int signo)
{
  abort_sim();
  errx(EXIT_FAILURE, "segmentation fault.");
}

int exec(Memory entry_p)
{
  Instruction insn;
  unsigned long clk = 0;

  uint32_t cmod;
  int memfd;

  unsigned int i;
  char c;

  if(signal(SIGINT, signal_on_sigint) == SIG_ERR) {
    err(EXIT_FAILURE, "signal SIGINT");
  }
  if(signal(SIGSEGV, signal_on_sigsegv) == SIG_ERR) {
    err(EXIT_FAILURE, "signal SIGSEGV");
  }

  /*
  for(i = 0; i < breakp_next; i++) {
    printf("Break point[%d]: 0x%08x\n", i, breakp[i]);
  }
  */

  opcode_table_init(opcode_t);

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
  next_PCR = ~0;
  KSPR = (Memory)STACK_DEFAULT;

  printf("Execution Start: entry = 0x%08x\n", PCR);

  do {
    /* choose stack */
    if(cmod != (PSR & PSR_CMOD_MASK)) {
      SPR = !cmod ? USPR : KSPR;
    }
    cmod = (PSR & PSR_CMOD_MASK);

    /* break point check */
    for(i = 0; i < breakp_next; i++) {
      if(PCR == breakp[i]) {
	step_by_step = true;
	break;
      }
    }

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

    /* decode */
    if(opcode_t[insn.base.opcode] == NULL) {
      print_instruction(insn);
      abort_sim();
      errx(EXIT_FAILURE, "invalid opcode. (pc:%08x op:%x)", PCR, insn.base.opcode);
    }

    if(DEBUG || step_by_step) {
      puts("---");
      print_instruction(insn);
    }

    /* execution */
    (*(opcode_t[insn.base.opcode]))(insn);

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
	write(memfd, memory_addr_phy2vm(memory_addr_virt2phy(0, false, false), 0x1000), false);
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
      /* alignment check */
      if(next_PCR & 0x3) {
	abort_sim();
	errx(EXIT_FAILURE, "invalid branch addres. %08x", next_PCR);
      }

      PCR = next_PCR;
      next_PCR = ~0;
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
