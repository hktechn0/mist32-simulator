#ifndef MIST32_LOAD_STORE_H
#define MIST32_LOAD_STORE_H

/* Load / Store wrapper */

#include <stdbool.h>

#include "mmu.h"
#include "vm.h"
#include "memory.h"
#include "cache.h"

union union_int32 {
  uint32_t u32;
  uint16_t u16[4];
  uint8_t u8[4];
};

/* Load */
static inline int memory_ld32(unsigned int *dest, Memory vaddr)
{
  Memory paddr;

  paddr = memory_addr_virt2phy(vaddr, false, false);
  if(memory_is_fault) return -1;

#if CACHE_L1_D_ENABLE
  *dest = memory_cache_l1_read(paddr, 0);
#else
  *dest = *(unsigned int *)memory_addr_phy2vm(paddr, false);
#endif

  return 0;
}

static inline int memory_ld16(unsigned int *dest, Memory vaddr)
{
  union union_int32 tmp;
  int e;
  /* FIXME: no error if byte access to MMIO area */
  if(!(e = memory_ld32(&tmp.u32, vaddr & 0xfffffffc))) {
    /* trick for little endian */
    *dest = tmp.u16[(~vaddr >> 1) & 1];
  }
  return e;
}

static inline int memory_ld8(unsigned int *dest, Memory vaddr)
{
  union union_int32 tmp;
  int e;
  /* FIXME: no error if byte access to MMIO area */
  if(!(e = memory_ld32(&tmp.u32, vaddr & 0xfffffffc))) {
    /* trick for little endian */
    *dest = tmp.u8[~vaddr & 3];
  }
  return e;
}

/* Store */
static inline int memory_st32(Memory vaddr, unsigned int src)
{
  Memory paddr;

  paddr = memory_addr_virt2phy(vaddr, true, false);
  if(memory_is_fault) return -1;

#if CACHE_L1_I_ENABLE || CACHE_L1_D_ENABLE
  memory_cache_l1_write(paddr, src);
#endif
#if !CACHE_L1_D_ENABLE
  *(unsigned int *)memory_addr_phy2vm(paddr, true) = src;
#endif

  return 0;
}

static inline int memory_st16(Memory vaddr, unsigned int src)
{
  Memory paddr;

  paddr = memory_addr_virt2phy(vaddr, true, false);
  if(memory_is_fault) return -1;

#if CACHE_L1_D_ENABLE
  union union_int32 tmp;

  tmp.u32 = *(unsigned int *)memory_addr_phy2vm(paddr & 0xfffffffc, false);
  tmp.u16[(~vaddr >> 1) & 1] = (unsigned short)src;
  /* FIXME: no error if byte access to MMIO area */
  memory_cache_l1_write(paddr & 0xfffffffc, tmp.u32);
#else
#if CACHE_L1_I_ENABLE
  memory_cache_l1_write(paddr, src);
#endif
  /* XOR for little endian */
  *(unsigned short *)memory_addr_phy2vm(paddr ^ 2, true) = (unsigned short)src;
#endif

  return 0;
}

static inline int memory_st8(Memory vaddr, unsigned int src)
{
  Memory paddr;

  paddr = memory_addr_virt2phy(vaddr, true, false);
  if(memory_is_fault) return -1;

#if CACHE_L1_D_ENABLE
  union union_int32 tmp;

  tmp.u32 = *(unsigned int *)memory_addr_phy2vm(paddr & 0xfffffffc, false);
  tmp.u8[~vaddr & 3] = (unsigned char)src;
  /* FIXME: no error if byte access to MMIO area */
  memory_cache_l1_write(paddr & 0xfffffffc, tmp.u32);
#else
#if CACHE_L1_I_ENABLE
  memory_cache_l1_write(paddr, src);
#endif
  /* XOR for little endian */
  *(unsigned char *)memory_addr_phy2vm(paddr ^ 3, true) = (unsigned char)src;
#endif

  return 0;
}

#endif /* MIST32_LOAD_STORE_H */
