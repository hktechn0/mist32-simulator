/* PREFETCH_SIZE must be below page size */
#define PREFETCH_SIZE 64
#define PREFETCH_TAG 0xffffffc0
#define PREFETCH_MASK 0x0000003f

extern unsigned int prefetch_insn[PREFETCH_SIZE];
extern Memory prefetch_pc;

static inline unsigned int instruction_fetch(void)
{
  Memory phypc;

  if((PCR & PREFETCH_TAG) == prefetch_pc) {
    return prefetch_insn[(PCR & PREFETCH_MASK) >> 2];
  }

  /* instruction fetch */
  phypc = memory_addr_virt2phy(PCR, false, true);

  if(memory_is_fault) {
    return NOP_INSN;
  }

  /* prefetch */
  memcpy(prefetch_insn, memory_addr_phy2vm(phypc & PREFETCH_TAG, false), PREFETCH_SIZE);

  prefetch_pc = PCR & PREFETCH_TAG;
  return prefetch_insn[(PCR & PREFETCH_MASK) >> 2];
}

#if CACHE_L1_I_ENABLE
static inline unsigned int instruction_fetch_cache(void)
{
  Memory phypc;
  
  /* instruction fetch */
  phypc = memory_addr_virt2phy(PCR, false, true);
  
  if(memory_is_fault) {
    /* fault fetch */
    return NOP_INSN;
  }

  return memory_cache_l1_read(phypc, 1);
}
#endif

static inline void instruction_prefetch_flush(void)
{
#if CACHE_L1_I_ENABLE
  prefetch_pc = 0xffffffff;
#endif
}
