#include <stdbool.h>
#include <stdlib.h>
#include <err.h>

#define MEMORY_CALLOC 1

/* simulator virtual memory construct (not MMU VM) */
#define PAGE_SIZE (16384)        /* 2 ^ 14 */
#define PAGE_ENTRY_NUM (262144)  /* 2 ^ 18 */
#define PAGE_SIZE_IN_WORD (PAGE_SIZE >> 2)

#define PAGE_OFFSET_BIT_NUM (14) /* 16KB   */
#define PAGE_OFFSET_MASK 0x00003fff /* 14 bit */
#define PAGE_NUM_MASK 0x0003ffff /* 18 bit */
#define PAGE_INDEX_MASK (PAGE_NUM_MASK << PAGE_OFFSET_BIT_NUM)

// #define MEMP(addr, w) ((unsigned int *)memory_addr_get(addr, w))
/* for little endian */
// #define MEMP8(addr, w) ((unsigned char *)memory_addr_get(addr ^ 3, w))
// #define MEMP16(addr, w) ((unsigned short *)memory_addr_get(addr ^ 2, w))

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

union union_int32 {
  uint32_t u32;
  uint16_t u16[4];
  uint8_t u8[4];
};

/* simulator virtual memory PageEntry (not MMU VM) */
typedef struct _pageentry {
  bool valid;
  void *addr;
} PageEntry;

extern PageEntry page_table[PAGE_ENTRY_NUM];

extern int memory_is_fault;
extern Memory memory_io_writeback;

void memory_init(void);
void memory_free(void);

void *memory_addr_mmio(Memory paddr, bool is_write);
Memory memory_page_walk_L2(Memory vaddr, bool is_write, bool is_exec);
Memory memory_page_fault(Memory vaddr);
Memory memory_page_protection_fault(Memory vaddr);

void memory_vm_alloc(Memory paddr, unsigned int page_num);
void *memory_vm_memcpy(void *dest, const void *src, size_t n);
int memory_vm_memcmp(const void *s1, const void *s2, size_t n);
//void memory_vm_page_convert_endian(uint32_t *page);
void memory_vm_convert_endian(void);

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

/* Physical address to VM memory address */
static inline void *memory_addr_phy2vm(Memory paddr, bool is_write)
{
  unsigned int page_num;

  if(paddr >= MEMORY_MAX_ADDR) {
    /* memory mapped I/O */
    return memory_addr_mmio(paddr, is_write);
  }

  /* virtual memory */
  page_num = (paddr >> PAGE_OFFSET_BIT_NUM) & PAGE_NUM_MASK;

  if(!page_table[page_num].valid) {
    /* VM memory page fault */
    memory_vm_alloc(paddr, page_num);
  }

  return (char *)page_table[page_num].addr + (paddr & PAGE_OFFSET_MASK);
}

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

#include "cache.h"

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
