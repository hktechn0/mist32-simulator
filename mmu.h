#ifndef MIST32_MMU_H
#define MIST32_MMU_H

#include <stdlib.h>
#include <err.h>

#include "registers.h"

/* 4KB Page */
#define MMU_PAGE_INDEX_L1 0xffc00000
#define MMU_PAGE_INDEX_L2 0x003ff000
#define MMU_PAGE_OFFSET 0x0000fff
#define MMU_PAGE_OFFSET_PSE 0x003fffff
#define MMU_PAGE_NUM 0xfffff000

#define MMU_PTE_VALID 0x001
#define MMU_PTE_R 0x002
#define MMU_PTE_D 0x004
#define MMU_PTE_EX 0x008
#define MMU_PTE_PP 0x030
#define MMU_PTE_PP_RWXX 0x000
#define MMU_PTE_PP_RDXX 0x010
#define MMU_PTE_PP_RWRD 0x020
#define MMU_PTE_PP_RWRW 0x030
#define MMU_PTE_CD 0x040
#define MMU_PTE_G 0x080
#define MMU_PTE_PE 0x100

Memory memory_page_walk_L2(Memory vaddr, bool is_write, bool is_exec);
Memory memory_page_fault(Memory vaddr);
Memory memory_page_protection_fault(Memory vaddr);

static inline bool memory_check_privilege(uint32_t pte, bool is_write, bool is_exec)
{
  if(is_exec && !(pte & MMU_PTE_EX)) {
    return false;
  }

  switch(pte & MMU_PTE_PP) {
  case MMU_PTE_PP_RWXX:
    if((PSR & PSR_CMOD_MASK) == PSR_CMOD_KERNEL) {
      return true;
    }
    break;
  case MMU_PTE_PP_RDXX:
    if((PSR & PSR_CMOD_MASK) == PSR_CMOD_KERNEL && !is_write) {
      return true;
    }
    break;
  case MMU_PTE_PP_RWRD:
    if((PSR & PSR_CMOD_MASK) == PSR_CMOD_KERNEL || !is_write) {
      return true;
    }
    break;
  case MMU_PTE_PP_RWRW:
    return true;
    break;
  default:
    break;
  }

  return false;
}

/* TLB definitions */
#include "tlb.h"

/* Get Physical address */
static inline Memory memory_addr_virt2phy(Memory vaddr, bool is_write, bool is_exec)
{
#if TLB_ENABLE
  Memory paddr;
#endif

  switch(PSR_MMUMOD) {
  case PSR_MMUMOD_DIRECT:
    /* Direct mode */
    return vaddr;
    break;
  case PSR_MMUMOD_L2:
    /* 2-Level Paging Mode */
#if TLB_ENABLE
    if((paddr = memory_tlb_get(vaddr, is_write, is_exec)) != MEMORY_MAX_ADDR) {
      /* TLB hit */
      return paddr;
    }
#endif
    return memory_page_walk_L2(vaddr, is_write, is_exec);
    break;
  default:
    errx(EXIT_FAILURE, "MMU mode (%d) not supported.", PSR_MMUMOD);
  }

  /* will not reach here */
  return MEMORY_MAX_ADDR;
}

#endif /* MIST32_MMU_H */
