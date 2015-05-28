#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common.h"
#include "debug.h"
#include "registers.h"
#include "vm.h"
#include "load_store.h"
#include "insn_format.h"

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

void debug_load_hw(Memory addr, unsigned int data)
{
  DEBUGLDHW("[L], %08x, %08x, %08x, %08x\n", PCR, SPR, addr, data);
  DEBUGLDPHY("[L], %08x(%08x), %08x(%08x), %08x(%08x), %08x\n",
	     PCR, memory_addr_virt2phy(PCR, false, false),
	     SPR, memory_addr_virt2phy(SPR, false, false),
	     addr, memory_addr_virt2phy(addr, false, false), data);
}

void debug_store_hw(Memory addr, unsigned int data)
{
  DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, addr, data);
  DEBUGSTPHY("[S], %08x(%08x), %08x(%08x), %08x(%08x), %08x\n",
	     PCR, memory_addr_virt2phy(PCR, false, false),
	     SPR, memory_addr_virt2phy(SPR, false, false),
	     addr, memory_addr_virt2phy(addr, false, false), data);
}

void debug_load8(Memory addr, unsigned char data)
{
  DEBUGLD("[Load ] Addr: 0x%08x, Data:       0x%02x, PC: 0x%08x\n", addr, data, PCR);
  debug_load_hw(addr, data);
}

void debug_load16(Memory addr, unsigned short data)
{
  DEBUGLD("[Load ] Addr: 0x%08x, Data:     0x%04x, PC: 0x%08x\n", addr, data, PCR);
  debug_load_hw(addr, data);
}

void debug_load32(Memory addr, unsigned int data)
{
  DEBUGLD("[Load ] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", addr, data, PCR);
  debug_load_hw(addr, data);
}

void debug_store8(Memory addr, unsigned char data)
{
  DEBUGST("[Store] Addr: 0x%08x, Data:       0x%02x, PC: 0x%08x\n", addr, data, PCR);
  debug_store_hw(addr, data);
}

void debug_store16(Memory addr, unsigned short data)
{
  DEBUGST("[Store] Addr: 0x%08x, Data:     0x%04x, PC: 0x%08x\n", addr, data, PCR);
  debug_store_hw(addr, data);
}

void debug_store32(Memory addr, unsigned int data)
{
  DEBUGST("[Store] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", addr, data, PCR);
  debug_store_hw(addr, data);
}

void debug_push(Memory addr, unsigned int data)
{
  DEBUGST("[Push ] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", addr, data, PCR);
  debug_store_hw(addr, data);
}

void debug_pop(Memory addr, unsigned int data)
{
  DEBUGLD("[Pop  ] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", addr, data, PCR);
  debug_load_hw(addr, data);
}
