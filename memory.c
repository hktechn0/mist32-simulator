#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include "common.h"

PageEntry *page_table;

void memory_init(void)
{
  page_table = calloc(PAGE_ENTRY_NUM, sizeof(PageEntry));
}

void memory_free(void)
{
  int i;
  PageEntry *entry;
  
  for(i = 0; i < PAGE_ENTRY_NUM; i++) {
    entry = &page_table[i];
    if(entry->valid) {
      free(entry->addr);
    }
  }

  free(page_table);
}

void *memory_addr_get(Memory addr)
{
  return memory_page_addr(addr) + (addr & PAGE_OFFSET_MASK);
}

void *memory_page_addr(Memory addr)
{
  PageEntry *entry;
  unsigned int page_num;

  page_num = (addr & PAGE_INDEX_MASK) >> PAGE_OFFSET_BIT_NUM;
  
  entry = &page_table[page_num];
  if(!entry->valid) {
    if((entry->addr = malloc(PAGE_SIZE)) == NULL) {
      err(EXIT_FAILURE, "page_alloc");
    }
    entry->valid = true;

    DPUTS("[Memory] alloc: Virt 0x%08x, Real 0x%08p on 0x%08x\n",
	  entry->addr, addr & PAGE_INDEX_MASK, addr);
  }

  return entry->addr;
}

void memory_convert_endian(void)
{
  unsigned int i;
    
  for(i = 0; i < PAGE_ENTRY_NUM; i++) {
    if(page_table[i].valid) {
      page_convert_endian(page_table[i].addr);
    }
  }
}

/* convert endian. must pass real address */
void page_convert_endian(unsigned int *page)
{
  unsigned int i;
  unsigned int *value;

  for(i = 0; i < PAGE_SIZE_IN_WORD; i++) {
    value = page + i;
    *value = (*value >> 24) | (*value << 24) | ((*value >> 8) & 0xff00) | ((*value << 8) & 0xff0000);
  }
}
