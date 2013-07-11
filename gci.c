#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "common.h"
#include "monitor.h"

gci_hub_info *gci_hub;
gci_hub_node *gci_hub_nodes;
gci_node gci_nodes[4];

char fifo_scancode[KMC_FIFO_SCANCODE_SIZE];
unsigned int fifo_scancode_start, fifo_scancode_end;

int fd_dispchar;

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
  fd_dispchar = open(FIFO_DISPLAY_CHAR, O_WRONLY);

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

  free((void *)gci_hub);
}

void gci_info(void)
{
  int i;

  printf("---- GCI ----\n");
  printf("[IOSR      ] 0x%08x\n", IOSR);
  printf("[GCI Hub   ] Size: %08x, Total: %d\n", gci_hub->space_size, gci_hub->total);

  for(i = 0; i < gci_hub->total; i++) {
    printf("[GCI Node %d] Size: %08x, Priority: %u\n", i, gci_hub_nodes[i].size, gci_hub_nodes[i].priority);
  }
}

/* GCI Device Emulation */

/* KMC */
void gci_kmc_read(Memory addr, Memory offset, void *mem)
{
  unsigned int *p;

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
  unsigned int *vram;
  char chr;

  /* escape sequence */
  char esc_clear[] = { 0x1b, 'c' };
  char esc_buf[100];

  esc_buf[0] = 0x1b;

  vram = mem;

  if(offset < GCI_DISPLAY_CHAR_SIZE) {
    /* character display mode */
    if(DEBUG_IO) {
      c = *(unsigned int *)((char *)vram + offset);
      chr = c & 0x7f;

      printf("[I/O] DISPLAY CHAR: '%c' (%02x) at %dx%d\n", chr, chr,
	     offset % (DISPLAY_CHAR_WIDTH * 4), offset / (DISPLAY_CHAR_WIDTH * 4));
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

	/* output char */
	write(fd_dispchar, &chr, 1);
      }

      chr = '\n';
      write(fd_dispchar, &chr, 1);
    }
  }
  else {
    /* bitmap display mode */
    c = *(unsigned int *)((char *)vram + offset);

    p = (offset - GCI_DISPLAY_CHAR_SIZE) / 4;
    x = p % DISPLAY_WIDTH;
    y = p / DISPLAY_WIDTH;

    DPUTS("[I/O] DISPLAY BITMAP %3dx%3d %x\n", x, y, c);

    monitor_display_draw(x, y, c);
  }
}
