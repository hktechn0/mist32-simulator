#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "interrupt.h"

idt_entry idt_cache[IDT_ENTRY_MAX];

/* Previous system registers */
struct FLAGS PFLAGR;
Memory PPCR;
unsigned int PPSR;
/* unsigned int PPDTR; */
/* unsigned int PTIDR; */

void interrupt_dispatcher(void)
{
  if(!(PSR & PSR_IM_ENABLE)) {
    /* interrupt disabled */
    return;
  }

  if(dps_utim64_interrupt()) {
    /* DPS UTIM64 */
    interrupt_entry(IDT_DPS_UTIM64_NUM);
  }
  else if(gci_kmc_interrupt()) {
    /* GCI KMC */
    interrupt_entry(IDT_GCI_KMC_NUM);
  }
  else if(dps_sci_interrupt()) {
    /* DPS LS */
    interrupt_entry(IDT_DPS_LS_NUM);
  }
}

void interrupt_entry(unsigned int num)
{
  if(!(idt_cache[num].flags & IDT_FLAGS_VALID)) {
    return;
  }

  PFLAGR = FLAGR;
  PPCR = PCR;
  PPSR = PSR;

  /* interrupt disable */
  PSR &= ~PSR_IM_ENABLE;

  /* entry interrupt */
  PCR = idt_cache[num].handler;

  DEBUGINT("[INTERRUPT] IRQ %x\n", num);
}

void interrupt_exit(void)
{
  PSR = PPSR;
  FLAGR = PFLAGR;

  next_PCR = PPCR;

  DEBUGINT("[INTERRUPT] IRQ Exit\n");
}

void interrupt_idt_store(void)
{
  memcpy((void *)idt_cache, MEMP(IDTR), IDT_ENTRY_MAX * sizeof(idt_entry));

  DEBUGINT("[INTERRUPT] IDT Store\n");
}
