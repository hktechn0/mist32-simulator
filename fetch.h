/* PREFETCH_SIZE must be below page size */
#define PREFETCH_SIZE 64
#define PREFETCH_N (PREFETCH_SIZE >> 2)
#define PREFETCH_TAG 0xffffffc0
#define PREFETCH_MASK 0x0000003f

extern uint32_t prefetch_insn[PREFETCH_N];
extern Memory prefetch_pc;

static inline uint32_t instruction_fetch(Memory pc)
{
  Memory phypc;

  uint32_t *dest;
  const uint32_t *src;

  if((pc & PREFETCH_TAG) == prefetch_pc) {
    return prefetch_insn[(pc & PREFETCH_MASK) >> 2];
  }

  /* instruction fetch */
  phypc = memory_addr_virt2phy(pc & PREFETCH_TAG, false, true);

  if(memory_is_fault) {
    return NOP_INSN;
  }

  /* prefetch, DO NOT USE memcpy() for endian mistake */
  dest = prefetch_insn;
  src = memory_addr_phy2vm(phypc, false);
  while(dest < prefetch_insn + PREFETCH_N) {
    *dest++ = *src++;
  }

  prefetch_pc = pc & PREFETCH_TAG;

  return prefetch_insn[(pc & PREFETCH_MASK) >> 2];
}

#if CACHE_L1_I_ENABLE
static inline uint32_t instruction_fetch_cache(Memory pc)
{
  Memory phypc;
  
  /* instruction fetch */
  phypc = memory_addr_virt2phy(pc, false, true);
  
  if(memory_is_fault) {
    /* fault fetch */
    return NOP_INSN;
  }

  return memory_cache_l1_read(phypc, 1);
}
#endif

static inline void instruction_prefetch_flush(void)
{
#if !CACHE_L1_I_ENABLE
  prefetch_pc = 0xffffffff;
#endif
}
