#include <stdbool.h>
#include <stdlib.h>
#include <err.h>

/* VM virtual memory != MMU */
#define PAGE_SIZE (16384)        /* 2 ^ 14 */
#define PAGE_ENTRY_NUM (262144)  /* 2 ^ 18 */
#define PAGE_SIZE_IN_WORD (PAGE_SIZE >> 2)

#define PAGE_OFFSET_BIT_NUM (14) /* 16KB   */
#define PAGE_OFFSET_MASK 0x00003fff /* 14 bit */
#define PAGE_NUM_MASK 0x0003ffff /* 18 bit */
#define PAGE_INDEX_MASK (PAGE_NUM_MASK << PAGE_OFFSET_BIT_NUM)

#define MEMP(addr) ((unsigned int *)memory_addr_get(addr))
/* for little endian */
#define MEMP8(addr) ((unsigned char *)memory_addr_get(addr ^ 3))
#define MEMP16(addr) ((unsigned short *)memory_addr_get(addr ^ 2))

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
#define MMU_PTE_CD 0x040
#define MMU_PTE_G 0x080
#define MMU_PTE_PE 0x100

typedef struct _pageentry {
  bool valid;
  void *addr;
} PageEntry;

extern PageEntry *page_table;

void memory_init(void);
void memory_free(void);

void *memory_addr_get_nonmemory(Memory addr);
void *memory_addr_get_L2page(Memory addr);
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
static inline void *memory_addr_get_from_physical(Memory addr)
{
  if(addr < MEMORY_MAX_ADDR) {
    /* virtual memory */
    return (char *)memory_page_addr(addr) + (addr & PAGE_OFFSET_MASK);
  }

  /* memory mapped I/O */
  return memory_addr_get_nonmemory(addr);
}

/* Virtual or Physical addr to VM memory addr */
static inline void *memory_addr_get(Memory addr)
{
  if(PSR_MMUMOD == PSR_MMUMOD_DIRECT) {
    /* Direct mode */
    return memory_addr_get_from_physical(addr);
  }
  else if(PSR_MMUMOD == PSR_MMUMOD_L2) {
    /* 2-Level Paging Mode */
    return memory_addr_get_L2page(addr);
  }
  else {
    errx(EXIT_FAILURE, "MMU mode (%d) not supported.", PSR_MMUMOD);
  }
}
