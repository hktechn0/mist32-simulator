#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include <endian.h>

#include "common.h"
#include "interrupt.h"

PageEntry *page_table;
TLB memory_tlb[TLB_ENTRY_MAX];

int memory_is_fault;
Memory memory_io_writeback;

/* unsigned int tlb_access = 0, tlb_hit = 0; */

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

/*
#if TLB_ENABLE
  printf("[TLB] Info hit %d / %d\n", tlb_hit, tlb_access);
#endif
*/

  free(page_table);
}

void *memory_addr_get_nonmemory(Memory addr, bool is_write)
{
  if(addr >= IOSR) {
    /* memory mapped IO area */
    if(addr & 0x3) {
      abort_sim();
      errx(EXIT_FAILURE, "Invalid alignment in IO area. Must be word align.");
    }

    if(is_write) {
      /* set io address for writeback */
      memory_io_writeback = addr;
    }
    else {
      io_load(addr);
    }

    return io_addr_get(addr);
  }
  else if(addr >= MEMORY_MAX_ADDR) {
    abort_sim();
    errx(EXIT_FAILURE, "No memory at %08x", addr);
  }

  return NULL;
}

void *memory_addr_get_L2page(Memory addr, bool is_write)
{
  unsigned int *pdt, *pt, pte;
  unsigned int index_l1, index_l2, offset, phyaddr;

#if TLB_ENABLE
  /* check TLB */
  do {
    unsigned int i, xoraddr;

    i = TLB_INDEX(addr);
    /* tlb_access++; */

    pte = memory_tlb[i].page_entry;

    if(!(pte & MMU_PTE_VALID)) {
      /* miss */
      continue;
    }

    xoraddr = memory_tlb[i].page_num ^ addr;

    if(xoraddr & MMU_PAGE_INDEX_L1) {
      /* miss */
      continue;
    }

    if(pte & MMU_PTE_PE) {
      /* Page Size Extension */
      phyaddr = (pte & MMU_PAGE_INDEX_L1) | (addr & MMU_PAGE_OFFSET_PSE);
    }
    else if(!(xoraddr & MMU_PAGE_NUM)) {
      phyaddr = (pte & MMU_PAGE_NUM) | (addr & MMU_PAGE_OFFSET);
    }
    else {
      /* miss */
      continue;
    }

    if(memory_check_privilege(pte, is_write)) {
      /* tlb_hit++; */
      return memory_addr_get_from_physical(phyaddr, is_write);
    }
    else {
      if(DEBUG_MMU) abort_sim();
      DEBUGMMU("[MMU] PAGE ACCESS DENIED TLB at 0x%08x (%08x)\n", addr, pte);

      return memory_page_protection_fault(addr);
    }
  } while(0);
#endif

  /* Level 1 */
  pdt = memory_addr_get_from_physical(PDTR, false);
  index_l1 = (addr & MMU_PAGE_INDEX_L1) >> 22;
  pte = pdt[index_l1];

  if(!(pte & MMU_PTE_VALID)) {
    /* Page Fault */
    if(DEBUG_MMU) abort_sim();
    DEBUGMMU("[MMU] PAGE FAULT L1 at 0x%08x (%08x)\n", addr, pte);

    return memory_page_fault(addr);
  }

  /* L1 privilege */
  if(!memory_check_privilege(pte, is_write)) {
    if(DEBUG_MMU) abort_sim();
    DEBUGMMU("[MMU] PAGE ACCESS DENIED L1 at 0x%08x (%08x)\n", addr, pte);

    return memory_page_protection_fault(addr);
  }

  if(pte & MMU_PTE_PE) {
    /* Page Size Extension */
#if TLB_ENABLE
    memory_tlb[TLB_INDEX(addr)].page_num = addr;
    memory_tlb[TLB_INDEX(addr)].page_entry = pte;
#endif

    offset = addr & MMU_PAGE_OFFSET_PSE;
    phyaddr = (pte & MMU_PAGE_INDEX_L1) | offset;
    return memory_addr_get_from_physical(phyaddr, is_write);
  }

  /* Level 2 */
  pt = memory_addr_get_from_physical(pte & MMU_PAGE_NUM, false);
  index_l2 = (addr & MMU_PAGE_INDEX_L2) >> 12;
  pte = pt[index_l2];

  if(!(pte & MMU_PTE_VALID)) {
    /* Page Fault */
    if(DEBUG_MMU) abort_sim();
    DEBUGMMU("[MMU] PAGE FAULT L2 at 0x%08x (%08x)\n", addr, pte);

    return memory_page_fault(addr);
  }

  /* L2 privilege */
  if(!memory_check_privilege(pte, is_write)) {
    if(DEBUG_MMU) abort_sim();
    DEBUGMMU("[MMU] PAGE ACCESS DENIED L2 at 0x%08x (%08x)\n", addr, pte);

    return memory_page_protection_fault(addr);
  }

#if TLB_ENABLE
  /* add TLB */
  memory_tlb[TLB_INDEX(addr)].page_num = addr;
  memory_tlb[TLB_INDEX(addr)].page_entry = pte;
#endif

  offset = addr & MMU_PAGE_OFFSET;
  phyaddr = (pte & MMU_PAGE_NUM) | offset;
  return memory_addr_get_from_physical(phyaddr, is_write);
}

void *memory_page_fault(Memory addr)
{
  memory_is_fault = IDT_PAGEFAULT_NUM;
  FI0R = addr;

  return NULL;
}

void *memory_page_protection_fault(Memory addr)
{
  memory_is_fault = IDT_INVALID_PRIV_NUM;
  FI0R = addr;

  return NULL;
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
