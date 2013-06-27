#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include "common.h"

void *dps;

gci_hub_info *gci_hub;
dps_utim64 *utim64a, *utim64b;
dps_sci *sci;

FILE *sci_rxd, *sci_txd;

void dps_init(void)
{
  dps = calloc(1, DPS_SIZE);

  utim64a = (void *)((char *)dps + DPS_UTIM64A);
  utim64b = (void *)((char *)dps + DPS_UTIM64B);
  sci = (void *)((char *)dps + DPS_SCI);

  sci_txd = fopen(FIFO_SCI_TXD, "w");
  sci_rxd = fopen(FIFO_SCI_RXD, "r");

  iosr -= DPS_SIZE;
}

void dps_close(void)
{
  fclose(sci_txd);
  fclose(sci_rxd);

  free(dps);
}

void gci_init(void)
{
  gci_hub = calloc(1, GCI_HUB_SIZE);

  gci_hub->total = 0;
  gci_hub->space_size = GCI_HUB_SIZE;

  iosr -= GCI_HUB_SIZE;
}

void gci_close(void)
{
  free((void *)gci_hub);
}

void io_init(void)
{
  DPUTS("[I/O] Initialize... ");

  iosr = 0;
  dps_init();
  gci_init();

  DPUTS("OK.\n");
}

void io_close(void)
{
  gci_close();
  dps_close();
}

void *io_addr_get(Memory addr)
{
  Memory offset, p;

  if(iosr > addr) {
    err(EXIT_FAILURE, "io_load");
  }

  offset = addr - iosr;
  
  if(offset < DPS_SIZE) {
    /* DPS */
    DPUTS("[I/O] DPS: Addr 0x%08x\n", offset);
    return (char *)dps + offset;
  }
  else if(offset < DPS_SIZE + GCI_HUB_SIZE) {
    /* GCI Hub */
    p = offset - DPS_SIZE;
    DPUTS("[I/O] GCI Hub: Addr 0x%08x\n", p);
    return (char *)gci_hub + p;
  }

  return NULL;
}

void io_load(Memory addr)
{
  char c;

  /* word align */
  addr &= ~0x03;

  if(addr == iosr + DPS_SCIRXD) {
    if((sci->cfg & SCICFG_REN) && (c = fgetc(sci_rxd)) != EOF) {
      sci->rxd = ((unsigned int)c & 0xff) | SCIRXD_VALID;
    }
    else {
      sci->rxd = 0;
    }
  }
}

void io_store(Memory addr)
{
  /* word align */
  addr &= ~0x03;
  
  if(addr == iosr + DPS_SCITXD && sci->cfg & SCICFG_TEN) {
    fputc(sci->txd & 0xff, sci_txd);
    fflush(sci_txd);
  }
}

void io_info(void)
{
  printf("---- I/O ----\n");
  printf("[SCI] TXD: %02x '%c' RXD: %d_%02x '%c' CFG: %08x\n",
	 sci->txd, sci->txd, sci->rxd & SCIRXD_VALID, sci->rxd, sci->rxd, sci->cfg);
  printf("[UTIM64] A: %d B: %d\n", utim64a->mcfg & UTIM64MCFG_ENA, utim64b->mcfg & UTIM64MCFG_ENA);
  printf("  A CC0: %08x CC1 %08x CC2 %08x CC3 %08x\n",
	 utim64a->cc0[1], utim64a->cc1[1], utim64a->cc2[1], utim64a->cc3[1]);
  printf("       : %08x     %08x     %08x     %08x\n",
	 utim64a->cc0cfg, utim64a->cc1cfg, utim64a->cc2cfg, utim64a->cc3cfg);
  printf("  B CC0: %08x CC1 %08x CC2 %08x CC3 %08x\n",
	 utim64b->cc0[1], utim64b->cc1[1], utim64b->cc2[1], utim64b->cc3[1]);
  printf("       : %08x     %08x     %08x     %08x\n",
	 utim64b->cc0cfg, utim64b->cc1cfg, utim64b->cc2cfg, utim64b->cc3cfg);
}
