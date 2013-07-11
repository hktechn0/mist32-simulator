#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "common.h"

void *dps;

dps_utim64 *utim64a, *utim64b;
dps_sci *sci;

int fd_scitxd, fd_scirxd;

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

  IOSR -= DPS_SIZE;
}

void dps_close(void)
{
  close(fd_scitxd);
  close(fd_scirxd);

  free(dps);
}

void dps_info(void)
{
  printf("---- DPS ----\n");
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

/* DPS Device Emulation */

void dps_sci_rxd_read(Memory addr, Memory offset)
{
  char c;

  if((sci->cfg & SCICFG_REN) && read(fd_scirxd, &c, 1) > 0) {
    sci->rxd = ((unsigned int)c & 0xff) | SCIRXD_VALID;
  }
  else {
    sci->rxd = 0;
  }
}

void dps_sci_txd_write(Memory addr, Memory offset)
{
  char c;

  if(sci->cfg & SCICFG_TEN) {
    c = sci->txd & 0xff;
    write(fd_scitxd, &c, 1);
  }
}
