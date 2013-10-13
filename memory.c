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
  if(addr >= IOSR) {
    /* memory mapped IO area */
    return io_addr_get(addr);
  }
  else if(addr >= MEMORY_MAX_ADDR) {
    errx(EXIT_FAILURE, "no memory at %08x", addr);
  }

  return NULL;
}

void *memory_addr_get_L2page(Memory addr)
{
  if(PSR_MMUPS != PSR_MMUPS_4KB) {
    errx(EXIT_FAILURE, "MMU page size (%d) not supported.", PSR_MMUPS);
  }

  /* Level 1 */
  pdt = memory_addr_get_from_physical(PDTR);
  index_l1 = (addr & MMU_PAGE_INDEX_L1) >> 22;

  if(!(pdt[index_l1] & MMU_PTE_VALID)) {
    /* Page Fault */
    errx(EXIT_FAILURE, "PAGE FAULT L1 at 0x%08x (%08x)", addr, pdt[index_l1]);
  }

  if(pdt[index_l1] & MMU_PTE_PE) {
    /* Page Size Extension */
    offset = addr & MMU_PAGE_OFFSET_PSE;
    return memory_addr_get_from_physical((pdt[index_l1] & MMU_PAGE_INDEX_L1) + offset);
  }

  /* Level 2 */
  pt = memory_addr_get_from_physical(pdt[index_l1] & MMU_PAGE_NUM);
  index_l2 = (addr & MMU_PAGE_INDEX_L2) >> 12;

  if(!(pt[index_l2] & MMU_PTE_VALID)) {
    /* Page Fault */
    errx(EXIT_FAILURE, "PAGE FAULT L2 at 0x%08x (%08x)", addr, pdt[index_l2]);
  }

  offset = addr & MMU_PAGE_OFFSET;
  return memory_addr_get_from_physical((pt[index_l2] & MMU_PAGE_NUM) + offset);
}

void memory_page_alloc(Memory addr, PageEntry *entry)
{
  if(!entry || entry->valid) {
    errx(EXIT_FAILURE, "page_alloc invalid entry");
  }
  else if((entry->addr = malloc(PAGE_SIZE)) == NULL) {
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
