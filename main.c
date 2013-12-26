#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <err.h>

#include <libelf.h>
#include <gelf.h>

#include "common.h"
#include "monitor.h"

bool DEBUG = false;
bool MONITOR = false;
bool DEBUG_LD = false, DEBUG_ST = false, DEBUG_JMP = false;
bool DEBUG_INT = false, DEBUG_MMU = false;
bool DEBUG_HW = false;

Memory breakp[100];
unsigned int breakp_next = 0;

int main(int argc, char **argv)
{
  unsigned int i, size, remaining;
  int opt;

  char *filename;
  int elf_fd;

  Elf *elf;
  Elf32_Ehdr *header;
  Elf_Scn *section;
  Elf32_Shdr *section_header;
  Elf_Data *data;
  Elf32_Addr section_addr, buffer_addr;

  GElf_Phdr phdr;
  Elf32_Addr paddr, vaddr;
  size_t phnum;

  void *allocp;

  while ((opt = getopt(argc, argv, "dvhmb:")) != -1) {
    switch (opt) {
    case 'd':
      /* debug mode */
      DEBUG = true;
    case 'v':
      /* verbose output */
      DEBUG_LD = true;
      DEBUG_ST = true;
      DEBUG_JMP = true;
      DEBUG_MMU = true;
      DEBUG_INT = true;
      break;
    case 'h':
      /* print load / store to compare RTL simulation */
      DEBUG_HW = true;
      break;
    case 'm':
      /* use monitor client */
      MONITOR = true;
      break;
    case 'b':
      /* break point */
      breakp[breakp_next++] = strtol(optarg, NULL, 0);
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s [-d] [-i] [-m] file\n",
	      argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "error: no input file\n");
    exit(EXIT_FAILURE);
  }
  else {
    /* ELF file */
    filename = argv[optind];
  }

  /* page table initialize */
  memory_init();

  if(MONITOR) {
    /* monitor initialize */
    monitor_init();
  }

  /* io initialize */
  io_init();

  /* set first section */
  section = 0;

  printf("---- Loading ELF ----\n");

  elf_version(EV_CURRENT);

  /* open ELF object file to exec */
  elf_fd = open(filename, O_RDONLY);
  elf = elf_begin(elf_fd, ELF_C_READ, NULL);

  if(elf_kind(elf) != ELF_K_ELF) {
    errx(EXIT_FAILURE, "'%s' is not an ELF object.", filename);
  }

  /* get ELF header */
  header = elf32_getehdr(elf);
  if(header->e_machine != EM_MIST32) {
    errx(EXIT_FAILURE, "'%s' is not for mist32.", filename);
  }

  /* check program header */
  if(elf_getphdrnum(elf, &phnum)) {
    errx(EXIT_FAILURE, "elf_getphdrnum() failed: %s.", elf_errmsg(-1));
  }

  paddr = 0;
  vaddr = 0;

  for (i = 0; i < phnum; i++) {
    gelf_getphdr(elf, i, &phdr);
    if(phdr.p_type == PT_LOAD) {
      if(phdr.p_vaddr != phdr.p_paddr) {
	vaddr = phdr.p_vaddr;
	paddr = phdr.p_paddr;
	printf("virtual:  0x%08x\n", vaddr);
	printf("physical: 0x%08x\n", paddr);
      }
    }
  }

  /* Load ELF object */
  while((section = elf_nextscn(elf, section)) != 0) {
    section_header = elf32_getshdr(section);

    /* Alloc section */
    if((section_header->sh_flags & SHF_ALLOC) && (section_header->sh_type != SHT_NOBITS)) {
      section_addr = section_header->sh_addr;
      buffer_addr = paddr + (section_addr - vaddr);

      printf("section: %s at 0x%08x on 0x%08x\n", 
	     elf_strptr(elf, header->e_shstrndx, section_header->sh_name), section_addr, buffer_addr);

      /* Load section data */
      data = NULL;
      while((data = elf_getdata(section, data)) != NULL) {
	printf("d_off: 0x%08x (%8d), d_size: 0x%08x (%8d)\n",
	       (unsigned int)data->d_off, (int)data->d_off,
	       (unsigned int)data->d_size, (int)data->d_size);

	/*
	for(i = 0; i < (data->d_size / 4); i++) {
	  printf("%08x\n", *(((unsigned int *)data->d_buf) + i));
	}
	*/

	/* Copy to virtual memory */
	for(i = 0, size = 0; i < data->d_size; i += size, buffer_addr += size) {
	  /* data size of remaining */
	  remaining = data->d_size - i;

	  /* destination page size of remaining */
	  size = PAGE_SIZE - (buffer_addr - (buffer_addr & PAGE_INDEX_MASK));

	  /* get real destination from virtual address */
	  allocp = memory_addr_phy2vm(buffer_addr, true);

	  if(size <= remaining) {
	    memcpy(allocp, (char *)data->d_buf + i, size);
	  }
	  else {
	    memcpy(allocp, (char *)data->d_buf + i, remaining);
	  }
	}
      }
    }
  }

  /* mist32 binary is big endian */
  memory_vm_convert_endian();

  printf("---- Start ----\n");

  /* Execute */
  exec((Memory)header->e_entry);

  /* clean up */
  if(MONITOR) {
    monitor_close();
  }

  io_close();
  memory_free();

  elf_end(elf);
  close(elf_fd);

  return 0;
}
