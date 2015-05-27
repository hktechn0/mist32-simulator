#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <err.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "common.h"
#include "debug.h"
#include "registers.h"
#include "io.h"
#include "dps.h"
#include "gci.h"
#include "monitor.h"

gci_hub_info *gci_hub;
gci_hub_node *gci_hub_nodes;
gci_node gci_nodes[4];

gci_mmcc *mmcc;

unsigned char fifo_scancode[KMC_FIFO_SCANCODE_SIZE];
unsigned int fifo_scancode_start, fifo_scancode_end;

int fd_dispchar, fd_mmcc;

void gci_init(void)
{
  int i;

  gci_hub = calloc(1, GCI_HUB_SIZE);
  gci_hub_nodes = (void *)((char *)gci_hub + GCI_HUB_HEADER_SIZE);

  /* initialize */
  gci_hub->total = 0;
  gci_hub->space_size = GCI_HUB_SIZE;

  /* STD-KMC */
  fifo_scancode_start = 0;
  fifo_scancode_end = 0;

  gci_nodes[GCI_KMC_NUM].node_info = calloc(1, GCI_NODE_SIZE);
  gci_nodes[GCI_KMC_NUM].device_area = calloc(1, GCI_KMC_AREA_SIZE);

  if(gci_nodes[GCI_KMC_NUM].node_info == NULL ||
     gci_nodes[GCI_KMC_NUM].device_area == NULL) {
    err(EXIT_FAILURE, "malloc STD-KMC");
  }

  gci_nodes[GCI_KMC_NUM].node_info->area_size = GCI_KMC_AREA_SIZE;
  gci_nodes[GCI_KMC_NUM].node_info->int_priority = GCI_KMC_INT_PRIORITY;
  gci_hub_nodes[GCI_KMC_NUM].size = GCI_NODE_SIZE + GCI_KMC_AREA_SIZE;
  gci_hub_nodes[GCI_KMC_NUM].priority = GCI_KMC_PRIORITY;

  gci_hub->space_size += gci_hub_nodes[GCI_KMC_NUM].size;

  /* STD-DISPLAY */
  fd_dispchar = open(FIFO_DISPLAY_CHAR_DEFAULT, O_WRONLY);
  /* FIXME: error check */

  gci_nodes[GCI_DISPLAY_NUM].node_info = calloc(1, GCI_NODE_SIZE);
  gci_nodes[GCI_DISPLAY_NUM].device_area = calloc(1, GCI_DISPLAY_AREA_SIZE);

  if(gci_nodes[GCI_DISPLAY_NUM].node_info == NULL ||
     gci_nodes[GCI_DISPLAY_NUM].device_area == NULL) {
    err(EXIT_FAILURE, "malloc STD-DISPLAY");
  }

  gci_nodes[GCI_DISPLAY_NUM].node_info->area_size = GCI_DISPLAY_AREA_SIZE;
  gci_nodes[GCI_DISPLAY_NUM].node_info->int_priority = GCI_DISPLAY_INT_PRIORITY;
  gci_hub_nodes[GCI_DISPLAY_NUM].size = GCI_NODE_SIZE + GCI_DISPLAY_AREA_SIZE;
  gci_hub_nodes[GCI_DISPLAY_NUM].priority = GCI_DISPLAY_PRIORITY;

  gci_hub->space_size += gci_hub_nodes[GCI_DISPLAY_NUM].size;

  /* STD-MMCC */
  if(gci_mmcc_image_file == NULL) {
    fd_mmcc = -1;
    DEBUGIO("[I/O] No MMC Image File\n");
  }
  else {
    fd_mmcc = open(gci_mmcc_image_file, O_RDWR);
    if(fd_mmcc == -1) {
      errx(EXIT_FAILURE, "Can't read MMC image file. %s\n", gci_mmcc_image_file);
    }
    DEBUGIO("[I/O] MMC Image '%s'\n", gci_mmcc_image_file);
  }

  gci_nodes[GCI_MMCC_NUM].node_info = calloc(1, GCI_NODE_SIZE);
  gci_nodes[GCI_MMCC_NUM].device_area = calloc(1, GCI_MMCC_AREA_SIZE);

  if(gci_nodes[GCI_MMCC_NUM].node_info == NULL ||
     gci_nodes[GCI_MMCC_NUM].device_area == NULL) {
    err(EXIT_FAILURE, "malloc STD-MMCC");
  }

  mmcc = gci_nodes[GCI_MMCC_NUM].device_area;

  gci_nodes[GCI_MMCC_NUM].node_info->area_size = GCI_MMCC_AREA_SIZE;
  gci_nodes[GCI_MMCC_NUM].node_info->int_priority = GCI_MMCC_INT_PRIORITY;
  gci_hub_nodes[GCI_MMCC_NUM].size = GCI_NODE_SIZE + GCI_MMCC_AREA_SIZE;
  gci_hub_nodes[GCI_MMCC_NUM].priority = GCI_MMCC_PRIORITY;

  gci_hub->space_size += gci_hub_nodes[GCI_MMCC_NUM].size;

  /* mprotect GCI Node Info */
  for(i = 0; i < GCI_NODE_MAX; i++) {
    if(gci_hub_nodes[i].size > 0) {
      gci_hub->total++;
      mprotect((void *)gci_nodes[i].node_info, GCI_NODE_SIZE, PROT_READ);
    }
  }

  IOSR -= gci_hub->space_size;
}

void gci_close(void)
{
  int i;

  for(i = 0; i < GCI_NODE_MAX; i++) {
    free((void *)gci_nodes[i].node_info);
    free(gci_nodes[i].device_area);
  }

  close(fd_dispchar);
  if(fd_mmcc != -1) {
    close(fd_mmcc);
  }

  free((void *)gci_hub);
}

void gci_info(void)
{
  int i;
  Memory addr;

  addr = IOSR + DPS_SIZE;

  DEBUGIO("---- GCI ----\n");
  DEBUGIO("[IOSR      ] 0x%08x\n", IOSR);
  DEBUGIO("[GCI Hub   ] 0x%08x Size: %08x, Total: %d\n", addr, gci_hub->space_size, gci_hub->total);

  addr += GCI_HUB_SIZE;

  for(i = 0; i < gci_hub->total; i++) {
    DEBUGIO("[GCI Node %d] 0x%08x Size: %08x, Priority: %u\n", i, addr, gci_hub_nodes[i].size, gci_hub_nodes[i].priority);
    addr += gci_hub_nodes[i].size;
  }
}

/* GCI Device Emulation */

/* KMC */
void gci_kmc_read(Memory addr, Memory offset, void *mem)
{
  uint32_t *p;

  p = gci_nodes[GCI_KMC_NUM].device_area;

  if(fifo_scancode_start == fifo_scancode_end) {
    /* FIFO empty */
    *p = 0;
  }
  else {
    *p = fifo_scancode[fifo_scancode_start++] | KMC_SCANCODE_VALID;

    if(fifo_scancode_start >= KMC_FIFO_SCANCODE_SIZE) {
      fifo_scancode_start = 0;
    }

    DEBUGIO("[I/O] KMC SCANCODE %x\n", *p & 0xff);
  }
}

bool gci_kmc_interrupt(void)
{
  if(gci_nodes[GCI_KMC_NUM].int_dispatch && !gci_nodes[GCI_KMC_NUM].int_issued) {
    gci_nodes[GCI_KMC_NUM].int_dispatch = false;
    gci_nodes[GCI_KMC_NUM].int_issued = true;
    return true;
  }

  return false;
}

/* DISPLAY */
void gci_display_write(Memory addr, Memory offset, void *mem)
{
  unsigned int c, fg, bg, r, g, b;
  unsigned int p, x, y;
  uint32_t *vram;
  char chr;

  /* escape sequence */
  char esc_clear[] = { 0x1b, 'c' };
  char esc_buf[100];

  esc_buf[0] = 0x1b;

  vram = mem;

  if(offset < GCI_DISPLAY_CHAR_SIZE) {
    /* character display mode */
    if(DEBUG_IO) {
      c = *(uint32_t *)((char *)vram + offset);
      chr = c & 0x7f;

      if(isprint(chr)) {
	DEBUGIO("[I/O] DISPLAY CHAR: '%c' (%02x) at %dx%d\n", chr, chr,
		offset % (DISPLAY_CHAR_WIDTH * 4), offset / (DISPLAY_CHAR_WIDTH * 4));
      }
      else {
	DEBUGIO("[I/O] DISPLAY CHAR: ??? (%02x) at %dx%d\n", chr,
		offset % (DISPLAY_CHAR_WIDTH * 4), offset / (DISPLAY_CHAR_WIDTH * 4));
      }
    }

    /* clear display */
    write(fd_dispchar, esc_clear, 2);

    for(y = 0; y < DISPLAY_CHAR_HEIGHT; y++) {
      for(x = 0; x < DISPLAY_CHAR_WIDTH; x++) {
	p = (y * (0x200 / 4)) + (x % DISPLAY_CHAR_WIDTH);
	c = vram[p];

	/* char code, char color, bg color */
	chr = c & 0x7f;
	fg = (c >> 8) & 0xfff;
	bg = (c >> 20) & 0xfff;

	if(chr == '\0') {
	  break;
	}

	r = (fg >> 8 & 0xf) / 2.7;
	g = (fg >> 4 & 0xf) / 2.7;
	b = (fg & 0xf) / 2.7;
	fg = (r * 36) + (g * 6) + (b * 1) + 16;

	r = (bg >> 8 & 0xf) / 2.7;
	g = (bg >> 4 & 0xf) / 2.7;
	b = (bg & 0xf) / 2.7;
	bg = (r * 36) + (g * 6) + (b * 1) + 16;

	/* color escape sequence */
	sprintf(esc_buf + 1, "[38;5;%dm", fg);
	write(fd_dispchar, esc_buf, strlen(esc_buf));

	sprintf(esc_buf + 1, "[48;5;%dm", bg);
	write(fd_dispchar, esc_buf, strlen(esc_buf));

	if(chr < 0x20 || 0x7e < chr) {
	  chr = ' ';
	}

	/* output char */
	write(fd_dispchar, &chr, 1);
      }

      chr = '\n';
      write(fd_dispchar, &chr, 1);
    }
  }
  else {
    /* bitmap display mode */
    c = *(uint32_t *)((char *)vram + offset);

    p = (offset - GCI_DISPLAY_CHAR_SIZE) / 4;
    x = p % DISPLAY_WIDTH;
    y = p / DISPLAY_WIDTH;

    DPUTS("[I/O] DISPLAY BITMAP %3dx%3d %x\n", x, y, c);

    if(MONITOR) {
      monitor_display_draw(x, y, c);
    }
  }
}

/* MMCC */
void gci_mmcc_read(Memory addr, Memory offset, void *mem)
{
  if(offset == GCI_MMCC_INIT_COMMAND) {
    errx(EXIT_FAILURE, "MMCC INIT_COMMAND not readable.");
  }
}

void gci_mmcc_write(Memory addr, Memory offset, void *mem)
{
  void *buf;
  uint32_t *value;

  if(fd_mmcc == -1) {
    errx(EXIT_FAILURE, "No MMC image.");
  }

  buf = ((char *)mmcc + MMCC_BUFFER_OFFSET);

  if(offset == GCI_MMCC_INIT_COMMAND) {
    DEBUGIO("[I/O] MMCC INIT_COMMAND\n");
  }
  else if(offset == GCI_MMCC_SECTOR_READ) {
    if(lseek(fd_mmcc, mmcc->sector_read, SEEK_SET) == -1) {
      errx(EXIT_FAILURE, "MMCC READ lseek");
    }
    if(read(fd_mmcc, buf, MMCC_SECTOR_SIZE) == -1) {
      errx(EXIT_FAILURE, "MMCC READ read");
    }

    for(value = buf; (char *)value < (char *)buf + MMCC_SECTOR_SIZE; value++) {
      // convert endian in buffer
      *value = __builtin_bswap32(*value);
    }

    DEBUGIO("[I/O] MMCC READ Sector: %d\n", mmcc->sector_read >> 9);
  }
  else if(offset == GCI_MMCC_SECTOR_WRITE) {
    // temporary buffer
    char writebuf[MMCC_SECTOR_SIZE];
    uint32_t *wbuf;

    if(lseek(fd_mmcc, mmcc->sector_write, SEEK_SET) == -1) {
      errx(EXIT_FAILURE, "MMCC WRITE lseek");
    }

    wbuf = (uint32_t *)writebuf;

    for(value = buf; (char *)value < (char *)buf + MMCC_SECTOR_SIZE; value++) {
      // convert endian in buffer
      *wbuf++ = __builtin_bswap32(*value);
    }

    if(write(fd_mmcc, writebuf, MMCC_SECTOR_SIZE) == -1) {
      errx(EXIT_FAILURE, "MMCC WRITE write");
    }

    DEBUGIO("[I/O] MMCC WRITE Sector: %d\n", mmcc->sector_write >> 9);
  }
}
