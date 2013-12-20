#include <stdbool.h>
#include <stdlib.h>
#include <err.h>

/* simulator virtual memory construct (not MMU VM) */
#define PAGE_SIZE (16384)        /* 2 ^ 14 */
#define PAGE_ENTRY_NUM (262144)  /* 2 ^ 18 */
#define PAGE_SIZE_IN_WORD (PAGE_SIZE >> 2)

#define PAGE_OFFSET_BIT_NUM (14) /* 16KB   */
#define PAGE_OFFSET_MASK 0x00003fff /* 14 bit */
#define PAGE_NUM_MASK 0x0003ffff /* 18 bit */
#define PAGE_INDEX_MASK (PAGE_NUM_MASK << PAGE_OFFSET_BIT_NUM)

/* simulator TLB settings */
#define TLB_ENABLE 1
#define TLB_ENTRY_MAX 16  /* must be 2^n */
#define TLB_INDEX_MASK (TLB_ENTRY_MAX - 1)
#define TLB_INDEX(addr) ((addr >> 22) & TLB_INDEX_MASK)

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

/* simulator virtual memory PageEntry (not MMU VM) */
typedef struct _pageentry {
  bool valid;
  void *addr;
} PageEntry;

extern PageEntry *page_table;

typedef struct _tlb {
  unsigned int page_num;
  unsigned int page_entry;
} TLB;

extern TLB memory_tlb[TLB_ENTRY_MAX];

extern int memory_is_fault;
extern Memory memory_io_writeback;

void memory_init(void);
void memory_free(void);

void *memory_addr_get_nonmemory(Memory addr, bool is_write);
void *memory_addr_get_L2page(Memory addr, bool is_write);
void *memory_page_fault(Memory addr);
void *memory_page_protection_fault(Memory addr);
void memory_page_alloc(Memory addr, PageEntry *entry);
void memory_convert_endian(void);

static inline void *memory_page_addr(Memory addr)
{
  PageEntry *entry;
  unsigned int page_num;

  page_num = (addr >> PAGE_OFFSET_BIT_NUM) & PAGE_NUM_MASK;
  entry = &page_table[page_num];

  if(!entry->valid) {
    /* VM memory page fault */
    memory_page_alloc(addr, entry);
  }

  return entry->addr;
}

/* Physical addr to VM memory addr */
static inline void *memory_addr_get_from_physical(Memory addr, bool is_write)
{
  if(addr < MEMORY_MAX_ADDR) {
    /* virtual memory */
    return (char *)memory_page_addr(addr) + (addr & PAGE_OFFSET_MASK);
  }

  /* memory mapped I/O */
  return memory_addr_get_nonmemory(addr, is_write);
}

/* Virtual or Physical addr to VM memory addr */
static inline void *memory_addr_get(Memory addr, bool is_write)
{
  switch(PSR_MMUMOD) {
  case PSR_MMUMOD_DIRECT:
    /* Direct mode */
    return memory_addr_get_from_physical(addr, is_write);
    break;
  case PSR_MMUMOD_L2:
    /* 2-Level Paging Mode */
    return memory_addr_get_L2page(addr, is_write);
    break;
  default:
    errx(EXIT_FAILURE, "MMU mode (%d) not supported.", PSR_MMUMOD);
    break;
  }

  return NULL;
}

static inline bool memory_check_privilege(unsigned int pte, bool is_write)
{
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

static inline void memory_tlb_flush(void)
{
#if TLB_ENABLE
  unsigned int i;

  for(i = 0; i < TLB_ENTRY_MAX; i++) {
    memory_tlb[i].page_entry = 0;
  }
#endif
}

static inline int memory_ld32(unsigned int *dest, Memory addr)
{
  unsigned int *tmp;

  tmp = (unsigned int *)memory_addr_get(addr, false);
  if(tmp == NULL) return -1;
  *dest = *tmp;

  return 0;
}

static inline int memory_ld16(unsigned int *dest, Memory addr)
{
  unsigned short *tmp;

  /* XOR for little endian */
  tmp = (unsigned short *)memory_addr_get(addr ^ 2, false);
  if(tmp == NULL) return -1;
  *dest = *tmp;

  return 0;
}

static inline int memory_ld8(unsigned int *dest, Memory addr)
{
  unsigned char *tmp;

  /* XOR for little endian */
  tmp = (unsigned char *)memory_addr_get(addr ^ 3, false);
  if(tmp == NULL) return -1;
  *dest = *tmp;

  return 0;
}

static inline int memory_st32(Memory addr, unsigned int src)
{
  unsigned int *tmp;

  tmp = (unsigned int *)memory_addr_get(addr, true);
  if(tmp == NULL) return -1;
  *tmp = src;

  return 0;
}

static inline int memory_st16(Memory addr, unsigned int src)
{
  unsigned short *tmp;

  /* XOR for little endian */
  tmp = (unsigned short *)memory_addr_get(addr ^ 2, true);
  if(tmp == NULL) return -1;
  *tmp = (unsigned short)src;

  return 0;
}

static inline int memory_st8(Memory addr, unsigned int src)
{
  unsigned char *tmp;

  /* XOR for little endian */
  tmp = (unsigned char *)memory_addr_get(addr ^ 3, true);
  if(tmp == NULL) return -1;
  *tmp = (unsigned char)src;

  return 0;
}
