#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "common.h"

void *dps;
dps_utim64 *utim64a, *utim64b;
dps_sci *sci;
unsigned int *dps_lsflags;

char fifo_sci_rx[SCI_FIFO_RX_SIZE];
unsigned int fifo_sci_rx_start, fifo_sci_rx_end;

int fd_scitxd, fd_scirxd;

bool dps_lsflags_clear;

void dps_init(void)
{
  unsigned int *p;

  dps = calloc(1, DPS_SIZE);

  /* UTIM 64 */
  utim64a = (void *)((char *)dps + DPS_UTIM64A);
  utim64b = (void *)((char *)dps + DPS_UTIM64B);

  /* SCI */
  sci = (void *)((char *)dps + DPS_SCI);
  fifo_sci_rx_start = 0;
  fifo_sci_rx_end = 0;
  fd_scitxd = open(FIFO_SCI_TXD, O_WRONLY);
  fd_scirxd = open(FIFO_SCI_RXD, O_RDONLY | O_NONBLOCK);

  /* MI */
  p = (void *)((char *)dps + DPS_MIMSR);
  *p = MEMORY_MAX_ADDR;
  mprotect(p, sizeof(int), PROT_READ);

  /* LSFLAGS */
  dps_lsflags = (void *)((char *)dps + DPS_LSFLAGS);

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
  DEBUGIO("---- DPS ----\n");
  DEBUGIO("[SCI] TXD: %02x '%c' RXD: %d_%02x '%c' CFG: %08x\n",
	  sci->txd, sci->txd, sci->rxd & SCIRXD_VALID, sci->rxd, sci->rxd, sci->cfg);
  DEBUGIO("[UTIM64] A: %d B: %d\n", utim64a->mcfg & UTIM64MCFG_ENA, utim64b->mcfg & UTIM64MCFG_ENA);
  DEBUGIO("  A: CC0 %08x CC1 %08x CC2 %08x CC3 %08x\n",
	  utim64a->cc0[1], utim64a->cc1[1], utim64a->cc2[1], utim64a->cc3[1]);
  DEBUGIO("CFG      %08x     %08x     %08x     %08x\n",
	  utim64a->cc0cfg, utim64a->cc1cfg, utim64a->cc2cfg, utim64a->cc3cfg);
  DEBUGIO("  B: CC0 %08x CC1 %08x CC2 %08x CC3 %08x\n",
	  utim64b->cc0[1], utim64b->cc1[1], utim64b->cc2[1], utim64b->cc3[1]);
  DEBUGIO("CFG      %08x     %08x     %08x     %08x\n",
	  utim64b->cc0cfg, utim64b->cc1cfg, utim64b->cc2cfg, utim64b->cc3cfg);
  DEBUGIO("[LSFLAGS] %x\n", *dps_lsflags);
}

/* DPS Device Emulation */

void dps_sci_rxd_read(Memory addr, Memory offset)
{
  char c;

  if(sci->cfg & SCICFG_REN && fifo_sci_rx_start != fifo_sci_rx_end) {
    c = fifo_sci_rx[fifo_sci_rx_start++];

    if(fifo_sci_rx_start >= SCI_FIFO_RX_SIZE) {
      fifo_sci_rx_start = 0;
    }

    sci->rxd = ((unsigned int)c & 0xff) | SCIRXD_VALID;

    DEBUGIO("[I/O] DPS SCI RXD 0x%02x\n", c);
  }
  else {
    /* FIFO Empty or Disabled receive */
    sci->rxd = 0;
  }
}

void dps_sci_txd_write(Memory addr, Memory offset)
{
  char c;

  if(sci->cfg & SCICFG_TEN) {
    c = sci->txd & 0xff;
    write(fd_scitxd, &c, 1);

    DEBUGIO("[I/O] DPS SCI TXD 0x%02x\n", c);
  }
}

void dps_sci_cfg_write(Memory addr, Memory offset)
{
  if(sci->cfg & SCICFG_TCLR) {
  }
  else if(sci->cfg & SCICFG_RCLR) {
    fifo_sci_rx_start = 0;
    fifo_sci_rx_end = 0;
    DEBUGIO("[I/O] DPS SCI Receive FIFO Cleared\n");
  }

  sci->cfg &= (~SCICFG_TCLR & ~SCICFG_RCLR);
}

bool dps_sci_interrupt(void)
{
  char buf[SCI_FIFO_RX_SIZE];
  int length, request, received;
  unsigned int rire;
  unsigned int i;

  if(dps_lsflags_clear) {
    *dps_lsflags = 0;
    dps_lsflags_clear = false;
  }

  if(!(sci->cfg & SCICFG_REN)) {
    return false;
  }

  rire = (sci->cfg & SCICFG_RIRE_MASK) >> SCICFG_RIRE_OFFSET;
  length = FIFO_USED(fifo_sci_rx_start, fifo_sci_rx_end, SCI_FIFO_RX_SIZE);
  request = SCI_FIFO_RX_SIZE - length - 1;

  received = read(fd_scirxd, buf, request);

  if(received > 0) {
    for(i = 0; i < received; i++) {
      fifo_sci_rx[fifo_sci_rx_end++] = buf[i];

      if(fifo_sci_rx_end >= SCI_FIFO_RX_SIZE) {
	fifo_sci_rx_end = 0;
      }

      DEBUGIO("[I/O] DPS SCI FIFO RXD %02x\n", buf[i]);
    }
  }
  else if(received == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
    /* error */
    err(EXIT_FAILURE, "sci_rxd");
  }

  if(rire && rire <= 0x4) {
    length += received;
    request = 1 << (rire - 1);

    if(length >= request && received > 0 && !(*dps_lsflags & DPS_LSFLAGS_SCIR)) {
      *dps_lsflags |= DPS_LSFLAGS_SCIR;
      return true;
    }
  }

  return false;
}

void dps_lsflags_read(Memory addr, Memory offset)
{
  dps_lsflags_clear = true;
}
