/* simulator TLB settings */
#define TLB_ENABLE 0
#define TLB_PROFILE 1

#define TLB_ENTRY_MAX 16  /* must be 2^n */
#define TLB_INDEX_MASK (TLB_ENTRY_MAX - 1)
#define TLB_INDEX(addr) ((addr >> 22) & TLB_INDEX_MASK)

typedef struct _tlb {
  uint32_t page_num;
  uint32_t page_entry;
} TLB;

extern TLB memory_tlb[TLB_ENTRY_MAX];
extern unsigned long long tlb_access, tlb_hit;

static inline void memory_tlb_flush(void)
{
#if TLB_ENABLE
  unsigned int i;

  for(i = 0; i < TLB_ENTRY_MAX; i++) {
    memory_tlb[i].page_entry = 0;
  }
#endif
}

static inline Memory memory_tlb_get(Memory vaddr, bool is_write, bool is_exec)
{
  unsigned int i;
  uint32_t xoraddr, pte;
  Memory paddr;

  i = TLB_INDEX(vaddr);

#if TLB_PROFILE
  tlb_access++;
#endif

  pte = memory_tlb[i].page_entry;

  if(!(pte & MMU_PTE_VALID)) {
    /* miss */
    return MEMORY_MAX_ADDR;
  }

  xoraddr = memory_tlb[i].page_num ^ vaddr;

  if(xoraddr & MMU_PAGE_INDEX_L1) {
    /* miss */
    return MEMORY_MAX_ADDR;
  }

  if(pte & MMU_PTE_PE) {
    /* Page Size Extension */
    paddr = (pte & MMU_PAGE_INDEX_L1) | (vaddr & MMU_PAGE_OFFSET_PSE);
  }
  else if(!(xoraddr & MMU_PAGE_NUM)) {
    paddr = (pte & MMU_PAGE_NUM) | (vaddr & MMU_PAGE_OFFSET);
  }
  else {
    /* miss */
    return MEMORY_MAX_ADDR;
  }

  if(!memory_check_privilege(pte, is_write, is_exec)) {
    /* privilege fault */
    return MEMORY_MAX_ADDR;
  }

#if TLB_PROFILE
  tlb_hit++;
#endif

  return paddr;
}
