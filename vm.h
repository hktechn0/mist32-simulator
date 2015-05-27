#ifndef MIST32_VM_H
#define MIST32_VM_H

#define MEMORY_CALLOC 1

/* simulator virtual memory construct (not MMU VM) */
#define PAGE_SIZE (16384)        /* 2 ^ 14 */
#define PAGE_ENTRY_NUM (262144)  /* 2 ^ 18 */
#define PAGE_SIZE_IN_WORD (PAGE_SIZE >> 2)

#define PAGE_OFFSET_BIT_NUM (14) /* 16KB   */
#define PAGE_OFFSET_MASK 0x00003fff /* 14 bit */
#define PAGE_NUM_MASK 0x0003ffff /* 18 bit */
#define PAGE_INDEX_MASK (PAGE_NUM_MASK << PAGE_OFFSET_BIT_NUM)

/* simulator virtual memory PageEntry (not MMU VM) */
typedef struct _pageentry {
  bool valid;
  void *addr;
} PageEntry;

extern PageEntry page_table[PAGE_ENTRY_NUM];

void memory_vm_alloc(Memory paddr, unsigned int page_num);
void *memory_vm_memcpy(void *dest, const void *src, size_t n);
int memory_vm_memcmp(const void *s1, const void *s2, size_t n);
//void memory_vm_page_convert_endian(uint32_t *page);
void memory_vm_convert_endian(void);

void *memory_addr_mmio(Memory paddr, bool is_write);

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

#endif /* MIST32_VM_H */
