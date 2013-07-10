#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "gci_device.h"
#include "interrupt.h"

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

  if(gci_kmc_interrupt()) {
    interrupt_entry(IDT_GCI_KMC_NUM);
  }
}

idt_entry *interrupt_vector_get(unsigned int num)
{
  idt_entry *p;

  p = (void *)MEMP(IDTR);

  return p + num;
}

void interrupt_entry(unsigned int num)
{
  idt_entry *idt;

  idt = interrupt_vector_get(num);

  if(!(idt->flags & IDT_FLAGS_VALID)) {
    return;
  }

  PFLAGR = FLAGR;
  PPCR = PCR;
  PPSR = PSR;

  /* interrupt disable */
  PSR &= ~PSR_IM_ENABLE;

  /* entry interrupt */
  PCR = idt->handler;

  DEBUGINT("[INTERRUPT] IRQ %x\n", num);
}

void interrupt_exit(void)
{
  PSR = PPSR;
  FLAGR = PFLAGR;

  next_PCR = PPCR;

  DEBUGINT("[INTERRUPT] IRQ Exit\n");
}
