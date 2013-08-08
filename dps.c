#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>

#include "common.h"

#define UTIM64_NAME(t) ((t == utim64a) ? 'A' : 'B')

void *dps;
dps_utim64 *utim64a, *utim64b;
dps_sci *sci;
unsigned int *dps_lsflags;

unsigned int *utim64_flags;
bool utim64_enable[2];
timer_t utim64a_timer[4], utim64b_timer[4];
bool utim64a_enable[4], utim64b_enable[4];
struct itimerspec utim64a_its[4], utim64b_its[4];

unsigned char fifo_sci_rx[SCI_FIFO_RX_SIZE];
unsigned int fifo_sci_rx_start, fifo_sci_rx_end;
int fd_scitxd, fd_scirxd;

bool dps_lsflags_clear, utim64_flags_clear;

void dps_init(void)
{
  struct sigaction sa;

  unsigned int *p;
  int i;

  dps = calloc(1, DPS_SIZE);

  /* UTIM 64 */
  utim64a = (void *)((char *)dps + DPS_UTIM64A);
  utim64b = (void *)((char *)dps + DPS_UTIM64B);
  utim64_flags = (void *)((char *)dps + DPS_UTIM64FLAGS);
  utim64_flags_clear = false;

  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = dps_utim64_timer_sigalrm;
  sigemptyset(&sa.sa_mask);
  if(sigaction(SIGALRM, &sa, NULL) == -1) {
    err(EXIT_FAILURE, "timer sigaction");
  }

  for(i = 0; i < 4; i++) {
    timer_create(CLOCK_REALTIME, NULL, &utim64a_timer[i]);
    timer_create(CLOCK_REALTIME, NULL, &utim64b_timer[i]);
  }

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
  dps_lsflags_clear = false;

  IOSR -= DPS_SIZE;
}

void dps_close(void)
{
  int i;

  for(i = 0; i < 4; i++) {
    timer_delete(utim64a_timer[i]);
    timer_delete(utim64b_timer[i]);
  }

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
	  utim64a->cc[0][1], utim64a->cc[1][1], utim64a->cc[2][1], utim64a->cc[3][1]);
  DEBUGIO("CFG      %08x     %08x     %08x     %08x\n",
	  utim64a->cccfg[0], utim64a->cccfg[1], utim64a->cccfg[2], utim64a->cccfg[3]);
  DEBUGIO("  B: CC0 %08x CC1 %08x CC2 %08x CC3 %08x\n",
	  utim64b->cc[0][1], utim64b->cc[1][1], utim64b->cc[2][1], utim64b->cc[3][1]);
  DEBUGIO("CFG      %08x     %08x     %08x     %08x\n",
	  utim64b->cccfg[0], utim64b->cccfg[1], utim64b->cccfg[2], utim64b->cccfg[3]);
  DEBUGIO("[LSFLAGS] %x\n", *dps_lsflags);
}

/* DPS Device Emulation */

/* UTIM64 */
void dps_utim64_cc_change(timer_t *timer, struct itimerspec *its, int num, bool start, bool stop)
{
  static const struct itimerspec its_stop = {{0, 0}, {0, 0}};

  if(start) {
    /* Timer Start */
    if(timer_settime(timer[num], 0, &its[num], NULL) == -1) {
      err(EXIT_FAILURE, "timer_settime start");
    }

    DEBUGIO("[I/O] UTIM64 CC%d Enable\n", num);
  }
  else if(stop) {
    /* Timer Stop */
    if(timer_settime(timer[num], 0, &its_stop, NULL) == -1) {
      err(EXIT_FAILURE, "timer_settime stop");
    }

    DEBUGIO("[I/O] UTIM64 CC%d Disable\n", num);
  }
}

void dps_utim64_read(Memory addr, Memory offset) {

  if(offset == DPS_UTIM64FLAGS) {
    /* FLAGS */
    utim64_flags_clear = true;
  }
  else if(offset == DPS_UTIM64A + DPS_UTIM64_MCR ||
	  offset == DPS_UTIM64B + DPS_UTIM64_MCR) {
    /* MCR not implemented */
    DEBUGIO("[WARN] utim64.mcr not implemented.\n");
  }
}

void dps_utim64_write(Memory addr, Memory offset)
{
  dps_utim64 *t;
  timer_t *timer;
  bool *tena, *ccena;
  struct itimerspec *its;
  Memory toffset;

  bool ena;
  unsigned int interval;
  int i, num;

  if(offset < DPS_UTIM64A + DPS_UTIM64_TIMER_SIZE) {
    /* Timer A */
    t = utim64a;
    timer = utim64a_timer;
    tena = &utim64_enable[0];
    ccena = utim64a_enable;
    its = utim64a_its;
    toffset = DPS_UTIM64A;
  }
  else if(offset < DPS_UTIM64B + DPS_UTIM64_TIMER_SIZE) {
    /* Timer B */
    t = utim64b;
    timer = utim64b_timer;
    tena = &utim64_enable[1];
    ccena = utim64b_enable;
    its = utim64b_its;
    toffset = DPS_UTIM64B;
  }
  else if(offset == DPS_UTIM64FLAGS) {
    /* FLAGS */
    errx(EXIT_FAILURE, "utim64_flags write");
  }
  else {
    return;
  }

  num = 0;

  switch(offset - toffset) {
  case DPS_UTIM64_MCFGR:
    /* ENA */
    ena = t->mcfg & UTIM64MCFG_ENA;

    if(!(*tena) && ena) {
      /* Timer Start */
      for(i = 0; i < 4; i++) {
	dps_utim64_cc_change(timer, its, i, (t->cccfg[i] & UTIM64CFG_ENA), false);
      }

      DEBUGIO("[I/O] UTIM64%c Start\n", UTIM64_NAME(t));
      dps_info();
    }
    else if(*tena && !ena) {
      /* Timer End */
      for(i = 0; i < 4; i++) {
	dps_utim64_cc_change(timer, its, i, false, (t->cccfg[i] & UTIM64CFG_ENA));
      }

      DEBUGIO("[I/O] UTIM64%c Stop\n", UTIM64_NAME(t));
      dps_info();
    }

    *tena = ena;
    break;

  case DPS_UTIM64_MCR:
  case DPS_UTIM64_MCR + 4:
    /* FIXME: Not implemented MCR */
    DEBUGIO("[WARN] utim64.mcr not implemented.\n");
    break;

  case DPS_UTIM64_CC3R:
  case DPS_UTIM64_CC3R + 4:
    num++;
  case DPS_UTIM64_CC2R:
  case DPS_UTIM64_CC2R + 4:
    num++;
  case DPS_UTIM64_CC1R:
  case DPS_UTIM64_CC1R + 4:
    num++;
  case DPS_UTIM64_CC0R:
  case DPS_UTIM64_CC0R + 4:
    /* CCxR */
    interval = t->cc[num][1] / 1024 / 48;

    /* vritual 1khz : real 49.1520mhz */
    its[num].it_interval.tv_sec = interval / 1000;
    its[num].it_interval.tv_nsec = interval % 1000 * 1000000;
    its[num].it_value.tv_sec = interval / 1000;
    its[num].it_value.tv_nsec = interval % 1000 * 1000000;

    DEBUGIO("[I/O] UTIM64%c CC%d Set (virt %d msec)\n", UTIM64_NAME(t), 0, interval);
    break;

  case DPS_UTIM64_CC3CFGR:
    num++;
  case DPS_UTIM64_CC2CFGR:
    num++;
  case DPS_UTIM64_CC1CFGR:
    num++;
  case DPS_UTIM64_CC0CFGR:
    /* CCCxFGR */
    ena = t->cccfg[num] & UTIM64CFG_ENA;
    dps_utim64_cc_change(timer, its, 0, (ena && !ccena[num] && *tena), (!ena && ccena[num] && *tena));
    ccena[num] = ena;
    break;
  }
}

bool dps_utim64_interrupt(void)
{
  if(utim64_flags_clear) {
    *utim64_flags = 0;
    utim64_flags_clear = false;
  }

  if(*utim64_flags) {
    return true;
  }

  return false;
}

void dps_utim64_timer_sigalrm(int sig, siginfo_t *si, void *uc)
{
  timer_t *tidp;
  int i;

  tidp = si->si_value.sival_ptr;

  for(i = 0; i < 4; i++) {
    if(tidp == utim64a_timer[i]) {
      *utim64_flags |= (1 << i);
      break;
    }
    else if(tidp == utim64b_timer[i]) {
      *utim64_flags |= (1 << (i + 4));
      break;
    }
  }
}

/* SCI */
void dps_sci_rxd_read(Memory addr, Memory offset)
{
  char c;

  if((sci->cfg & SCICFG_REN) && (fifo_sci_rx_start != fifo_sci_rx_end)) {
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
    /* receive module disabled */
    return false;
  }

  /* set fifo length and remaining */
  length = FIFO_USED(fifo_sci_rx_start, fifo_sci_rx_end, SCI_FIFO_RX_SIZE);
  request = SCI_FIFO_RX_SIZE - length - 1;

  /* read input */
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
  else if(received == -1) {
    if(errno != EAGAIN && errno != EWOULDBLOCK) {
      /* error */
      err(EXIT_FAILURE, "sci_rxd");
    }
    else {
      /* no data received */
      received = 0;
    }
  }

  rire = (sci->cfg & SCICFG_RIRE_MASK) >> SCICFG_RIRE_OFFSET;

  if(!rire && rire > 0x4) {
    /* interrupt disabled or invalid */
    return false;
  }

  length += received;
  request = 1 << (rire - 1);

  if(length >= request && !(*dps_lsflags & DPS_LSFLAGS_SCIR)) {
    *dps_lsflags |= DPS_LSFLAGS_SCIR;
    return true;
  }

  return false;
}

void dps_lsflags_read(Memory addr, Memory offset)
{
  dps_lsflags_clear = true;
}
