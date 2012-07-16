#include <stdbool.h>

#define PAGE_SIZE (16384)        /* 2 ^ 14 */
#define PAGE_ENTRY_NUM (262144)  /* 2 ^ 14 */
#define PAGE_SIZE_IN_WORD (PAGE_SIZE / 4)

#define PAGE_OFFSET_BIT_NUM (14) /* 16KB   */
#define PAGE_OFFSET_MASK 0x0000003fff
#define PAGE_INDEX_MASK (~PAGE_OFFSET_MASK)

#define MEMP(addr) (memory_addr_get(addr))

typedef unsigned int Memory;

typedef struct _pageentry {
  void *addr;
  bool valid;
} PageEntry;

extern PageEntry *page_table;

void memory_init(void);
void memory_free(void);
void *memory_addr_get(Memory addr);
void *memory_page_addr(Memory addr);
void memory_convert_endian(void);
void page_convert_endian(unsigned int *page);
