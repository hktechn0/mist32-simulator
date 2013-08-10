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

int interrupt_nmi = -1;

void interrupt_entry(unsigned int num)
{
  /* interrupt vector is valid? */
  if(!IDT_ISVALID(num)) {
    DEBUGINT("[INTERRUPT] IRQ %x: invalid vector.\n", num);

    if(num == IDT_DOUBLEFAULT_NUM) {
      errx(EXIT_FAILURE, "Invalid double fault vector.");
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

void interrupt_dispatch_nonmask(unsigned int num)
{
  if(!IDT_ISVALID(num)) {
    /* invalid idt */
    interrupt_nmi = IDT_INVALID_IDT_NUM;
    DEBUGINT("[INTERRUPT] NMI %x invalid verctor.\n", num);
    return;
  }

  interrupt_nmi = num;

  if(!IDT_ISENABLE(num)) {
    DEBUGINT("[INTERRUPT] NMI %x [WARN] valid vector, but not enable.\n", num);
  }
  else {
    DEBUGINT("[INTERRUPT] NMI %x\n", num);
  }
}

void interrupt_idt_store(void)
{
  memcpy((void *)idt_cache, MEMP(IDTR), IDT_ENTRY_MAX * sizeof(idt_entry));

  DEBUGINT("[INTERRUPT] IDT Store\n");
}
