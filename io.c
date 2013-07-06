#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "common.h"
#include "gci_device.h"

void *dps;

gci_hub_info *gci_hub;
gci_hub_node *gci_hub_nodes;
gci_node gci_nodes[4];

dps_utim64 *utim64a, *utim64b;
dps_sci *sci;

int fd_scitxd, fd_scirxd, fd_dispchar;

void dps_init(void)
{
  unsigned int *p;

  dps = calloc(1, DPS_SIZE);

  /* UTIM 64 */
  utim64a = (void *)((char *)dps + DPS_UTIM64A);
  utim64b = (void *)((char *)dps + DPS_UTIM64B);

  /* SCI */
  sci = (void *)((char *)dps + DPS_SCI);
  fd_scitxd = open(FIFO_SCI_TXD, O_WRONLY);
  fd_scirxd = open(FIFO_SCI_RXD, O_RDONLY | O_NONBLOCK);

  /* MI */
  p = (void *)((char *)dps + DPS_MIMSR);
  *p = MEMORY_MAX_ADDR;
  mprotect(p, sizeof(int), PROT_READ);

  /* LSFLAGS */
  mprotect((char *)dps + DPS_LSFLAGS, sizeof(int), PROT_READ);

  iosr -= DPS_SIZE;
}

void dps_close(void)
{
  close(fd_scitxd);
  close(fd_scirxd);

  free(dps);
}

void gci_init(void)
{
  int i;

  gci_hub = calloc(1, GCI_HUB_SIZE);
  gci_hub_nodes = (void *)((char *)gci_hub + GCI_HUB_HEADER_SIZE);

  /* initialize */
  gci_hub->total = 0;
  gci_hub->space_size = GCI_HUB_SIZE;

  /* STD-KMC */
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

  iosr -= gci_hub->space_size;
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

void io_init(void)
{
  DPUTS("[I/O] Initialize... ");

  iosr = 0;
  dps_init();
  gci_init();

  DPUTS("OK.\n");

  gci_info();
}

void io_close(void)
{
  gci_close();
  dps_close();
}

void *io_addr_get(Memory addr)
{
  Memory offset, p;
  int i;

  if(iosr > addr) {
    err(EXIT_FAILURE, "io_load");
  }

  offset = addr - iosr;
  
  if(offset < DPS_SIZE) {
    /* DPS */
    DPUTS("[I/O] DPS: Addr: 0x%08x\n", offset);
    return (char *)dps + offset;
  }
  else if(offset < DPS_SIZE + GCI_HUB_SIZE) {
    /* GCI Hub */
    p = offset - DPS_SIZE;
    DPUTS("[I/O] GCI Hub: Addr: 0x%08x\n", p);
    return (char *)gci_hub + p;
  }
  else {
    p = DPS_SIZE + GCI_HUB_SIZE;

    for(i = 0; i < GCI_NODE_MAX; i++) {
      if(offset < p + GCI_NODE_SIZE) {
	/* GCI Node Info */
	DPUTS("[I/O] GCI Node Info: %d, Addr: 0x%08x\n", i, offset - p);
	return (char *)gci_nodes[i].node_info + (offset - p);
      }
      else if(offset < p + gci_hub_nodes[i].size) {
	/* GCI Device Area */
	DPUTS("[I/O] GCI Device: %d, Addr: 0x%08x\n", i, offset - (p + GCI_NODE_SIZE));
	return (char *)gci_nodes[i].device_area + offset - (p + GCI_NODE_SIZE);
      }
      else {
	/* next */
	p += gci_hub_nodes[i].size;
      }
    }
  }

  return NULL;
}

void io_load(Memory addr)
{
  Memory offset, p;
  char c;
  int i;

  /* word align */
  offset = addr & ~0x03;
  offset -= iosr;

  if(offset == DPS_SCIRXD) {
    if((sci->cfg & SCICFG_REN) && read(fd_scirxd, &c, 1) > 0) {
      sci->rxd = ((unsigned int)c & 0xff) | SCIRXD_VALID;
    }
    else {
      sci->rxd = 0;
    }
  }
  else if(offset > DPS_SIZE + GCI_HUB_SIZE) {
    /* GCI Area */
    p = DPS_SIZE + GCI_HUB_SIZE;

    for(i = 0; i < GCI_NODE_MAX; i++) {
      if(offset < p + GCI_NODE_SIZE) {
	/* Nothing to do if GCI Node Info */
	break;
      }
      else if(offset < p + gci_hub_nodes[i].size) {
	p += GCI_NODE_SIZE;

	switch(i) {
	case GCI_KMC_NUM:
	  gci_kmc_read(addr, offset - p, gci_nodes[i].device_area);
	  break;
	default:
	  break;
	}

	break;
      }
      else {
	/* next */
	p += gci_hub_nodes[i].size;
      }
    }
  }
}

void io_store(Memory addr)
{
  Memory offset, p;
  char c;
  int i;

  /* word align */
  offset = addr & ~0x03;
  offset -= iosr;

  if(offset == DPS_SCITXD && sci->cfg & SCICFG_TEN) {
    /* SCI TXD */
    c = sci->txd & 0xff;
    write(fd_scitxd, &c, 1);
  }
  else if(offset > DPS_SIZE + GCI_HUB_SIZE) {
    /* GCI Area */
    p = DPS_SIZE + GCI_HUB_SIZE;    

    for(i = 0; i < GCI_NODE_MAX; i++) {
      if(offset < p + GCI_NODE_SIZE) {
	/* Nothing to do if GCI Node Info */
	break;
      }
      else if(offset < p + gci_hub_nodes[i].size) {
	p += GCI_NODE_SIZE;

	switch(i) {
	case GCI_DISPLAY_NUM:
	  /* DISPLAY */
	  gci_display_write(addr, offset - p, gci_nodes[i].device_area);
	  break;
	default:
	  break;
	}

	break;
      }
      else {
	/* next */
	p += gci_hub_nodes[i].size;
      }
    }
  }
}

void io_info(void)
{
  printf("---- I/O ----\n");
  printf("[SCI] TXD: %02x '%c' RXD: %d_%02x '%c' CFG: %08x\n",
	 sci->txd, sci->txd, sci->rxd & SCIRXD_VALID, sci->rxd, sci->rxd, sci->cfg);
  printf("[UTIM64] A: %d B: %d\n", utim64a->mcfg & UTIM64MCFG_ENA, utim64b->mcfg & UTIM64MCFG_ENA);
  printf("  A: CC0 %08x CC1 %08x CC2 %08x CC3 %08x\n",
	 utim64a->cc0[1], utim64a->cc1[1], utim64a->cc2[1], utim64a->cc3[1]);
  printf("CFG      %08x     %08x     %08x     %08x\n",
	 utim64a->cc0cfg, utim64a->cc1cfg, utim64a->cc2cfg, utim64a->cc3cfg);
  printf("  B: CC0 %08x CC1 %08x CC2 %08x CC3 %08x\n",
	 utim64b->cc0[1], utim64b->cc1[1], utim64b->cc2[1], utim64b->cc3[1]);
  printf("CFG      %08x     %08x     %08x     %08x\n",
	 utim64b->cc0cfg, utim64b->cc1cfg, utim64b->cc2cfg, utim64b->cc3cfg);
}

void gci_info(void)
{
  int i;

  printf("---- GCI ----\n");
  printf("[IOSR      ] 0x%08x\n", iosr);
  printf("[GCI Hub   ] Size: %08x, Total: %d\n", gci_hub->space_size, gci_hub->total);

  for(i = 0; i < gci_hub->total; i++) {
    printf("[GCI Node %d] Size: %08x, Priority: %u\n", i, gci_hub_nodes[i].size, gci_hub_nodes[i].priority);
  }
}
