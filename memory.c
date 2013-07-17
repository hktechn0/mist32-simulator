#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include <endian.h>

#include "common.h"

PageEntry *page_table;

void memory_init(void)
{
  unsigned int i;

  page_table = calloc(PAGE_ENTRY_NUM, sizeof(PageEntry));

  for(i = 0; i < PAGE_ENTRY_NUM; i++) {
    page_table[i].valid = false;
  }
}

void memory_free(void)
{
  unsigned int i;
  PageEntry *entry;
  
  for(i = 0; i < PAGE_ENTRY_NUM; i++) {
    entry = &page_table[i];
    if(entry->valid) {
      free(entry->addr);
    }
  }

  free(page_table);
}

void *memory_addr_get_nonmemory(Memory addr)
{
  extern Memory IOSR;

  if(addr >= IOSR) {
    /* memory mapped IO area */
    return io_addr_get(addr);
  }
  else if(addr >= MEMORY_MAX_ADDR) {
    errx(EXIT_FAILURE, "no memory at %08x", addr);
  }

  return NULL;
}

void memory_page_alloc(Memory addr, PageEntry *entry)
{
  if((entry->addr = malloc(PAGE_SIZE)) == NULL) {
    err(EXIT_FAILURE, "page_alloc");
  }

  entry->valid = true;

  DPUTS("[Memory] alloc: Virt %p, Real 0x%08x on 0x%08x\n",
	entry->addr, addr & PAGE_INDEX_MASK, addr);
}

/* convert endian. must pass real address */
void memory_page_convert_endian(unsigned int *page)
{
  unsigned int i;
  unsigned int *value;

  value = page;

  for(i = 0; i < PAGE_SIZE_IN_WORD; i++) {
    /*
     *value = (*value >> 24) | (*value << 24) | ((*value >> 8) & 0xff00) | ((*value << 8) & 0xff0000);
     value++;
    */

    *value = htobe32(*value);
    value++;
  }
}

void memory_convert_endian(void)
{
  unsigned int i;
    
  for(i = 0; i < PAGE_ENTRY_NUM; i++) {
    if(page_table[i].valid) {
      memory_page_convert_endian(page_table[i].addr);
    }
  }
}
