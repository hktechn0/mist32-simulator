#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"

void print_instruction(Instruction insn)
{
  NOTICE("PC: 0x%08x Op: 0x%03x(%4d) Insn: 0x%08x SP: 0x%08x\n",
	 PCR, insn.base.opcode, insn.base.opcode, insn.value, SPR);
}

void print_registers(void)
{
  int i;

  NOTICE("PSR: %08x IDT: %08x PDT: %08x TID: %08x\n",
	 PSR, IDTR, PDTR, TIDR);
  NOTICE("KSP: %08x USP: %08x\n",
	 KSPR, USPR);
  NOTICE("ZF: %d, PF: %d, CF: %d, OF: %d, SF %d\n",
	 FLAGR.zero, FLAGR.parity, FLAGR.carry, FLAGR.overflow, FLAGR.sign);
  for(i = 0; i < 32; i++) {
    NOTICE("R%2d: 0x%08x (%11d) ", i, GR[i], GR[i]);
    if(!((i + 1) % 2)) { NOTICE("\n"); }
  }
}

void print_stack(Memory sp)
{
  unsigned int i;
  uint32_t data;

  printf("---- Stack ----\n");
  for(i = sp; i - sp < 40; i += 4) {
    if(i >= MEMORY_MAX_ADDR) { break; }

    if(memory_ld32(&data, i)) {
      /* if fault */
      break;
    }

    NOTICE("0x%08x: 0x%08x (%11d)\n", i, data, data);
  }
}

void abort_sim(void)
{
  printf("---- !!!! ABORT !!!! ----\n");
  printf("PCR: %08x SPR: %08x\n", PCR, SPR);
  print_registers();
}

#if !NO_DEBUG
void print_traceback(void)
{
  int i, n = 0;

  NOTICE("---- Traceback ----\n");
  for(i = traceback_next - 1; i >= 0; i--) {
    NOTICE("#%d\t0x%08x\n", n++, traceback[i]);
  }
}

void step_by_step_pause(void)
{
  int memfd;
  char c;

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
#endif
