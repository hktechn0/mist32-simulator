#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <err.h>

#include <elf.h>
#include <libelf.h>
#include <gelf.h>

#include "common.h"

bool DEBUG = false;
bool DEBUG_I = false;

int main(int argc, char **argv)
{
  unsigned int i, size, remaining;

  char *filename;
  int elf_fd;

  Elf *elf;
  Elf32_Ehdr *header;
  Elf_Scn *section;
  Elf32_Shdr *section_header;
  Elf_Data *data;
  Elf32_Addr section_addr, buffer_addr;

  void *allocp;

  if(argc <= 1) {
    puts("error: no input file");
    return 0;
  }

  if(argc == 3) {
    filename = argv[2];

    if(!strcmp(argv[1], "-d")) {
      /* debug mode */
      DEBUG = true;
    }
    else if(!strcmp(argv[1], "-di")) {
      /* interactive debug mode */
      DEBUG = true;
      DEBUG_I = true;
    }
  }
  else {
    filename = argv[1];
  }
  
  elf_version(EV_CURRENT);

  /* open ELF object file to exec */
  elf_fd = open(filename, O_RDONLY);
  elf = elf_begin(elf_fd, ELF_C_READ, NULL);

  if(elf_kind(elf) != ELF_K_ELF) {
    errx(EXIT_FAILURE, "'%s' is not an ELF object.", filename);
  }

  header = elf32_getehdr(elf);
  if(header->e_machine != EM_MIST32) {
    errx(EXIT_FAILURE, "'%s' is not for mist32.", filename);
  }

  /* page table initialize */
  memory_init();

  /* set first section */
  section = 0;

  /* Load ELF object */
  while((section = elf_nextscn(elf, section)) != 0) {
    section_header = elf32_getshdr(section);

    /* Alloc section */
    if((section_header->sh_flags & SHF_ALLOC) && (section_header->sh_type != SHT_NOBITS)) {
      section_addr = section_header->sh_addr;

      printf("section: %s at 0x%08x\n", 
	     elf_strptr(elf, header->e_shstrndx, section_header->sh_name), section_addr);

      /* Load section data */
      buffer_addr = section_addr;
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
	for(i = 0, size = 0; i < data->d_size; i += size) {
	  /* data size of remaining */
	  remaining = data->d_size - i;

	  /* destination page size of remaining */
	  size = PAGE_SIZE - (buffer_addr - (buffer_addr & PAGE_INDEX_MASK));

	  /* get real destination from virtual address */
	  allocp = MEMP(buffer_addr);

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
  memory_convert_endian();

  /* Execute */
  exec((Memory)header->e_entry);
  
  memory_free();
  elf_end(elf);
  close(elf_fd);

  return 0;
}
