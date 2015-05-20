#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "fetch.h"
#include "interrupt.h"

idt_entry idt_cache[IDT_ENTRY_MAX];

/* Previous system registers */
FLAGS PFLAGR;
Memory PPCR;
uint32_t PPSR;
Memory PPDTR;
uint32_t PTIDR;

int interrupt_nmi = -1;

void interrupt_entry(unsigned int num)
{
  /* interrupt vector is valid? */
  if(!IDT_ISVALID(num)) {
    DEBUGINT("[INTERRUPT] IRQ %x: invalid vector.\n", num);

    if(num == IDT_DOUBLEFAULT_NUM) {
      NOTICE("[INTERRUPT] Invalid double fault vector.\n");
      return_code = EXIT_FAILURE;
      exec_finish = true;
    }
    else if(num == IDT_INVALID_IDT_NUM) {
      interrupt_entry(IDT_DOUBLEFAULT_NUM);
    }
    else {
      interrupt_entry(IDT_INVALID_IDT_NUM);
    }

    return;
  }

  PFLAGR = FLAGR;
  PPCR = PCR;
  PPSR = PSR;
  PPDTR = PDTR;
  PTIDR = TIDR;

  /* interrupt disable, kernel mode */
  PSR &= (~PSR_IM_ENABLE & ~PSR_CMOD_MASK);

  /* entry interrupt */
  PCR = idt_cache[num].handler;

  DEBUGINT("[IRQ] %02x to %08x PSR: %08x KSP: %08x USP: %08x\n", num, PCR, PPSR, KSPR, USPR);
}

void interrupt_exit(void)
{
  if(PPDTR != PDTR || (PPSR & PSR_MMUMOD_MASK) != (PSR & PSR_MMUMOD_MASK)) {
    memory_tlb_flush();
    instruction_prefetch_flush();
  }

  FLAGR = PFLAGR;
  next_PCR = PPCR;
  PSR = PPSR;
  PDTR = PPDTR;
  TIDR = PTIDR;

  if(PSR_MMUMOD && PSR_MMUPS != PSR_MMUPS_4KB) {
    abort_sim();
    errx(EXIT_FAILURE, "MMU page size (%d) not supported.", PSR_MMUPS);
  }

  DEBUGINT("[IRQ] Exit %08x PSR: %08x KSP: %08x USP: %08x\n", PPCR, PPSR, KSPR, USPR);
  /* print_registers(); */
}

void interrupt_dispatch_nonmask(unsigned int num)
{
  if (TESTSUITE_MODE && num == IDT_SWIRQ_START_NUM) {
    /* SYS_exit */
    NOTICE("[INTERRUPT] SWI: SYS_exit (0x%x)\n", GR[2]);
    return_code = GR[2];
    exec_finish = true;
    return;
  }

  if(!IDT_ISVALID(num)) {
    /* invalid idt */
    interrupt_nmi = IDT_INVALID_IDT_NUM;
    DEBUGINT("[INTERRUPT] NMI %x: invalid verctor.\n", num);
    return;
  }

  interrupt_nmi = num;

  if(!IDT_ISENABLE(num)) {
    DEBUGINT("[INTERRUPT] NMI %x: [WARN] valid vector, but not enable.\n", num);
  }
  else {
    DEBUGINT("[INTERRUPT] NMI %x\n", num);
  }
}

void interrupt_idt_store(void)
{
  memory_vm_memcpy((void *)idt_cache, memory_addr_phy2vm(IDTR, false), IDT_ENTRY_MAX * sizeof(idt_entry));

  DEBUGINT("[INTERRUPT] IDT Store\n");
}
