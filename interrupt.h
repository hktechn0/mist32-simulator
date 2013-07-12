#define IDT_ENTRY_MAX 128

#define IDT_FLAGS_NONE 0x0
#define IDT_FLAGS_VALID 0x1
#define IDT_FLAGS_ENABLE 0x2

/* GCI */
#define IDT_GCI_START_NUM 5
#define IDT_GCI_KMC_NUM (IDT_GCI_START_NUM + GCI_KMC_NUM)
/* DPS */
#define IDT_DPS_UTIM64_NUM 36
#define IDT_DPS_LS_NUM 37
/* FAULT */
#define IDT_PAGEFAULT_NUM 40
#define IDT_INVALIDPRIV_NUM 41
#define IDT_INVALIDINST_NUM 42
#define IDT_INVALIDIDT_NUM 43
#define IDT_DIVERROR_NUM 40
/* ABORT */
#define IDT_DOUBLEFAULT_NUM 63
/* SOFTWARE IRQ */
#define IDT_SWIRQ_START_NUM 64

typedef volatile struct _idt_entry {
  unsigned int flags;
  Memory handler;
} idt_entry;

void interrupt_dispatcher(void);
void interrupt_entry(unsigned int num);
void interrupt_exit(void);
