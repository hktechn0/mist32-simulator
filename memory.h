#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
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

/* L1 Cache */
#define CACHE_L1_I_ENABLE 1
#define CACHE_L1_I_PROFILE 1
#define CACHE_L1_WAY 4
#define CACHE_L1_LINE_PER_WAY 16
#define CACHE_L1_LINE_SIZE 16 /* number of word */
#define CACHE_L1_LINE_MASK 0xffffffc0
#define CACHE_L1_TAG(addr) (addr & 0xfffffc00)
#define CACHE_L1_INDEX(addr) ((addr >> 6) & 0xf)
#define CACHE_L1_WORD(addr) ((addr >> 2) & 0xf)

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

typedef struct _cachelinel1 {
  bool valid;
  unsigned char miss;
  unsigned int tag;
  unsigned int line[CACHE_L1_LINE_SIZE];
} CacheLineL1;

extern CacheLineL1 cache_l1i[CACHE_L1_WAY][CACHE_L1_LINE_PER_WAY];
extern CacheLineL1 cache_l1d[CACHE_L1_WAY][CACHE_L1_LINE_PER_WAY];
extern unsigned long long cache_l1i_total, cache_l1i_hit;
extern unsigned long long cache_l1d_total, cache_l1d_hit;

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

void *memory_addr_mmio(Memory paddr, bool is_write);
Memory memory_page_walk_L2(Memory vaddr, bool is_write);
Memory memory_page_fault(Memory vaddr);
Memory memory_page_protection_fault(Memory vaddr);

void memory_vm_alloc(Memory paddr, PageEntry *entry);
void memory_vm_convert_endian(void);

/* Physical address to VM memory address */
static inline void *memory_addr_phy2vm(Memory paddr, bool is_write)
{
  PageEntry *entry;
  unsigned int page_num;

  if(paddr >= MEMORY_MAX_ADDR) {
    /* memory mapped I/O */
    return memory_addr_mmio(paddr, is_write);
  }

  /* virtual memory */
  page_num = (paddr >> PAGE_OFFSET_BIT_NUM) & PAGE_NUM_MASK;
  entry = &page_table[page_num];

  if(!entry->valid) {
    /* VM memory page fault */
    memory_vm_alloc(paddr, entry);
  }

  return (char *)entry->addr + (paddr & PAGE_OFFSET_MASK);
}

/* Get Physical address */
static inline Memory memory_addr_virt2phy(Memory vaddr, bool is_write)
{
  switch(PSR_MMUMOD) {
  case PSR_MMUMOD_DIRECT:
    /* Direct mode */
    return vaddr;
    break;
  case PSR_MMUMOD_L2:
    /* 2-Level Paging Mode */
    return memory_page_walk_L2(vaddr, is_write);
    break;
  default:
    errx(EXIT_FAILURE, "MMU mode (%d) not supported.", PSR_MMUMOD);
  }

  /* will not reach here */
  return MEMORY_MAX_ADDR;
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

static inline unsigned int memory_cache_l1i_read(Memory paddr)
{
  unsigned int w, i;
  unsigned int tag, index, word;
  unsigned int miss, maxmiss, target;

  tag = CACHE_L1_TAG(paddr);
  index = CACHE_L1_INDEX(paddr);
  word = CACHE_L1_WORD(paddr);

  for(w = 0; w < CACHE_L1_WAY; w++) {
    if(cache_l1i[w][index].tag == tag && cache_l1i[w][index].valid) {
      /* hit */
      for(i = 0; i < CACHE_L1_WAY; i++) {
	/* LRU */
	cache_l1i[i][index].miss++;
      }
      cache_l1i[w][index].miss = 0;

#if CACHE_L1_I_PROFILE
      cache_l1i_total++;
      cache_l1i_hit++;
#endif

      return cache_l1i[w][index].line[word];
    }
  }

  /* miss */
  maxmiss = 0;

#if CACHE_L1_I_PROFILE
      cache_l1i_total++;
#endif

  /* find victim by LRU */
  for(w = 0; w < CACHE_L1_WAY; w++) {
    if(!cache_l1i[w][index].valid) {
      target = w;
      break;
    }

    miss = cache_l1i[w][index].miss;
    if(maxmiss < miss) {
      maxmiss = miss;
      target = w;
    }
  }

  cache_l1i[target][index].valid = true;
  cache_l1i[target][index].miss = 0;
  cache_l1i[target][index].tag = tag;
  memcpy(&cache_l1i[target][index].line,
	 memory_addr_phy2vm(paddr & CACHE_L1_LINE_MASK, false),
	 CACHE_L1_LINE_SIZE * sizeof(unsigned int));

  return cache_l1i[target][index].line[word];
}

static inline bool memory_cache_l1i_write(Memory paddr)
{
  unsigned int w, tag, index;

  tag = CACHE_L1_TAG(paddr);
  index = CACHE_L1_INDEX(paddr);

  for(w = 0; w < CACHE_L1_WAY; w++) {
    if(cache_l1i[w][index].tag == tag) {
      cache_l1i[w][index].valid = false;
      return true;
    }
  }

  return false;
}

static inline int memory_ld32(unsigned int *dest, Memory vaddr)
{
  Memory paddr;

  paddr = memory_addr_virt2phy(vaddr, false);
  if(memory_is_fault) return -1;
  *dest = *(unsigned int *)memory_addr_phy2vm(paddr, false);

  return 0;
}

static inline int memory_ld16(unsigned int *dest, Memory vaddr)
{
  unsigned short tmp[2];
  int e;
  if(!(e = memory_ld32((unsigned int *)&tmp, vaddr & 0xfffffffc))) {
    /* trick for little endian */
    *dest = tmp[(~vaddr >> 1) & 1];
  }
  return e;
}

static inline int memory_ld8(unsigned int *dest, Memory vaddr)
{
  unsigned char tmp[4];
  int e;
  if(!(e = memory_ld32((unsigned int *)&tmp, vaddr & 0xfffffffc))) {
    /* trick for little endian */
    *dest = tmp[~vaddr & 3];
  }
  return e;
}

static inline int memory_st32(Memory vaddr, unsigned int src)
{
  Memory paddr;

  paddr = memory_addr_virt2phy(vaddr, true);
  if(memory_is_fault) return -1;
  *(unsigned int *)memory_addr_phy2vm(paddr, true) = src;

#if CACHE_L1_I_ENABLE
  memory_cache_l1i_write(paddr);
#endif

  return 0;
}

static inline int memory_st16(Memory vaddr, unsigned int src)
{
  Memory paddr;

  paddr = memory_addr_virt2phy(vaddr, true);
  if(memory_is_fault) return -1;
  /* XOR for little endian */
  *(unsigned short *)memory_addr_phy2vm(paddr ^ 2, true) = (unsigned short)src;

#if CACHE_L1_I_ENABLE
  memory_cache_l1i_write(paddr);
#endif

  return 0;
}

static inline int memory_st8(Memory vaddr, unsigned int src)
{
  Memory paddr;

  paddr = memory_addr_virt2phy(vaddr, true);
  if(memory_is_fault) return -1;
  /* XOR for little endian */
  *(unsigned char *)memory_addr_phy2vm(paddr ^ 3, true) = (unsigned char)src;

#if CACHE_L1_I_ENABLE
  memory_cache_l1i_write(paddr);
#endif

  return 0;
}
