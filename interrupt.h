#ifndef MIST32_INTERRUPT_H
#define MIST32_INTERRUPT_H

#include "gci.h"

/* Flags */
#define IDT_FLAGS_NONE 0x0
#define IDT_FLAGS_VALID 0x1
#define IDT_FLAGS_ENABLE 0x2

#define IDT_ENTRY_MAX 128

/* GCI */
#define IDT_GCI_START_NUM 5
#define IDT_GCI_KMC_NUM (IDT_GCI_START_NUM + GCI_KMC_NUM)
/* DPS */
#define IDT_DPS_UTIM64_NUM 36
#define IDT_DPS_LS_NUM 37
/* FAULT */
#define IDT_PAGEFAULT_NUM 40
#define IDT_INVALID_PRIV_NUM 41
#define IDT_INVALID_INST_NUM 42
#define IDT_INVALID_IDT_NUM 43
#define IDT_DIVERROR_NUM 40
/* ABORT */
#define IDT_DOUBLEFAULT_NUM 63
/* SOFTWARE IRQ */
#define IDT_SWIRQ_START_NUM 64

/* TYPE-E Device */
#define IDT_TYPEE_KEYBOARD_NUM 4
#define IDT_TYPEE_SCI_NUM 5
#define IDT_TYPEE_MMC_NUM 6

/* Check IDT */
#define IDT_ISVALID(num)			\
  (idt_cache[num].flags & IDT_FLAGS_VALID)

#define IDT_ISENABLE(num)				\
  (IDT_ISVALID(num)					\
   && (idt_cache[num].flags & IDT_FLAGS_ENABLE))

/* IDT Entry struct */
typedef volatile struct _idt_entry {
  unsigned int flags;
  Memory handler;
} idt_entry;

extern idt_entry idt_cache[IDT_ENTRY_MAX];
extern int interrupt_nmi;

/* interrupt.c */
void interrupt_entry(unsigned int num);
void interrupt_exit(void);
void interrupt_dispatch_nonmask(unsigned int num);
void interrupt_idt_store(void);

#endif /* MIST32_INTERRUPT_H */
