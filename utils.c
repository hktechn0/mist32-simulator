#include <stdio.h>
#include "common.h"

void print_instruction(Instruction insn)
{
  printf("PC: 0x%08x Op: 0x%03x(%4d) Insn: 0x%08x SP: 0x%08x\n",
	 PCR, insn.base.opcode, insn.base.opcode, insn.value, SPR);
}

void print_registers(void)
{
  unsigned int i;  
  printf("PSR: %08x IDT: %08x PDT: %08x TID: %08x\n",
	 PSR, IDTR, PDTR, TIDR);
  printf("KSP: %08x USP: %08x\n",
	 KSPR, USPR);
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

    if(memory_ld32(&data, i)) {
      /* if fault */
      break;
    }

    printf("0x%08x: 0x%08x (%11d)\n", i, data, data);
  }
}

void print_traceback(void)
{
  int i, n = 0;

  printf("---- Traceback ----\n");
  for(i = traceback_next - 1; i >= 0; i--) {
    printf("#%d\t0x%08x\n", n++, traceback[i]);
  }
}

void abort_sim(void)
{
  printf("---- !!!! ABORT !!!! ----\n");
  printf("PCR: %08x SPR: %08x\n", PCR, SPR);
  print_registers();
}
