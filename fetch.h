#ifndef MIST32_FETCH_H
#define MIST32_FETCH_H

#include "common.h"
#include "mmu.h"
#include "vm.h"
#include "memory.h"
#include "cache.h"
#include "utils.h"

/* PREFETCH_SIZE must be below page size */
#define PREFETCH_SIZE 64
#define PREFETCH_N (PREFETCH_SIZE >> 2)
#define PREFETCH_TAG 0xffffffc0
#define PREFETCH_MASK 0x0000003f

#define NOP_INSN (0x100 << 21)

#if CACHE_L1_I_ENABLE
static inline uint32_t instruction_fetch(Memory pc)
{
  Memory phypc;

  phypc = memory_addr_virt2phy(pc, false, true);

  if(memory_is_fault) {
    /* fault fetch */
    return NOP_INSN;
  }

  /* instruction fetch from cache */
  return memory_cache_l1_read(phypc, 1);
}

static inline void instruction_prefetch_flush(void)
{
  // NOTHING TO DO
}

#else
extern Memory prefetch_pc;
extern uint32_t prefetch_insn[PREFETCH_N];

static inline uint32_t instruction_fetch(Memory pc)
{
  Memory phypc;

  uint64_t *dest;
  const uint64_t *src;

  /* prefetch hit */
  if((pc & PREFETCH_TAG) == prefetch_pc) {
    return prefetch_insn[(pc & PREFETCH_MASK) >> 2];
  }

  phypc = memory_addr_virt2phy(pc & PREFETCH_TAG, false, true);

  if(memory_is_fault) {
    /* fault fetch */
    return NOP_INSN;
  }

  prefetch_pc = pc & PREFETCH_TAG;
  dest = (uint64_t *)prefetch_insn;
  src = memory_addr_phy2vm(phypc, false);

  /* prefetch, DO NOT USE memcpy() for endian mistake */
  while(dest < (uint64_t *)(prefetch_insn + PREFETCH_N)) {
    *dest++ = *src++;
  }

  /* DO NOT REMOVE THIS.
     prefetch_insn[] to be broken. */
  mem_barrier();

  return prefetch_insn[(pc & PREFETCH_MASK) >> 2];
}

static inline void instruction_prefetch_flush(void)
{
  prefetch_pc = 0xffffffff;
}
#endif

#endif /* MIST32_FETCH_H */
