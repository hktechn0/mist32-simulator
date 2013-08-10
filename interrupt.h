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
#define IDT_INVALID_PRIV_NUM 41
#define IDT_INVALID_INST_NUM 42
#define IDT_INVALID_IDT_NUM 43
#define IDT_DIVERROR_NUM 40
/* ABORT */
#define IDT_DOUBLEFAULT_NUM 63
/* SOFTWARE IRQ */
#define IDT_SWIRQ_START_NUM 64

#define IDT_ISVALID(num)			\
  (idt_cache[num].flags & IDT_FLAGS_VALID)

#define IDT_ISENABLE(num)				\
  ((idt_cache[num].flags & IDT_FLAGS_VALID)		\
   && (idt_cache[num].flags & IDT_FLAGS_ENABLE))

typedef volatile struct _idt_entry {
  unsigned int flags;
  Memory handler;
} idt_entry;

extern idt_entry idt_cache[IDT_ENTRY_MAX];
extern int interrupt_nmi;

void interrupt_entry(unsigned int num);
void interrupt_exit(void);
void interrupt_dispatch_nonmask(unsigned int num);
void interrupt_idt_store(void);

/* check interrupt coming in */
static inline void interrupt_dispatcher(void)
{
  int sci_int;

  sci_int = dps_sci_interrupt();

  if(interrupt_nmi != -1) {
    /* Non-maskable interrupt */
    interrupt_entry(interrupt_nmi);
    interrupt_nmi = -1;
  }

  if(!(PSR & PSR_IM_ENABLE)) {
    /* interrupt disabled */
    return;
  }

  if(IDT_ISENABLE(IDT_DPS_UTIM64_NUM) && dps_utim64_interrupt()) {
    /* DPS UTIM64 */
    interrupt_entry(IDT_DPS_UTIM64_NUM);
  }
  else if(IDT_ISENABLE(IDT_GCI_KMC_NUM) && gci_kmc_interrupt()) {
    /* GCI KMC */
    interrupt_entry(IDT_GCI_KMC_NUM);
  }
  else if(IDT_ISENABLE(IDT_DPS_LS_NUM) && sci_int) {
    /* DPS LS */
    interrupt_entry(IDT_DPS_LS_NUM);
  }
}
