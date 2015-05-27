#ifndef MIST32_DPS_H
#define MIST32_DPS_H

#include <signal.h>

#define DPS_SIZE 0x200

#define DPS_UTIM64A 0x000
#define DPS_UTIM64B 0x040
#define DPS_UTIM64_SIZE 0x7f
#define DPS_UTIM64_TIMER_SIZE 0x3c

#define DPS_UTIM64_MCFGR 0x000
#define DPS_UTIM64_MCR 0x004
#define DPS_UTIM64_CC0R 0x00c
#define DPS_UTIM64_CC1R 0x014
#define DPS_UTIM64_CC2R 0x01c
#define DPS_UTIM64_CC3R 0x024
#define DPS_UTIM64_CC0CFGR 0x02c
#define DPS_UTIM64_CC1CFGR 0x030
#define DPS_UTIM64_CC2CFGR 0x034
#define DPS_UTIM64_CC3CFGR 0x038

#define DPS_UTIM64FLAGS 0x07c

#define UTIM64MCFG_ENA 0x1
#define UTIM64CFG_ENA 0x1
#define UTIM64CFG_IE 0x2
#define UTIM64CFG_BIT 0x4
#define UTIM64CFG_MODE 0x8
#define UTIM64FLAGS_A0 0x01
#define UTIM64FLAGS_A1 0x02
#define UTIM64FLAGS_A2 0x04
#define UTIM64FLAGS_A3 0x08
#define UTIM64FLAGS_B0 0x10
#define UTIM64FLAGS_B1 0x20
#define UTIM64FLAGS_B2 0x40
#define UTIM64FLAGS_B3 0x80

#define DPS_SCI 0x100
#define DPS_SCITXD 0x100
#define DPS_SCIRXD 0x104
#define DPS_SCICFG 0x108

#define DPS_MIMSR 0x120
#define DPS_LSFLAGS 0x1fc
#define DPS_LSFLAGS_SCITIE 0x01
#define DPS_LSFLAGS_SCIRIE 0x02

#define SCIRXD_VALID 0x80000000
#define SCICFG_TEN 0x1
#define SCICFG_REN 0x2
#define SCICFG_TCLR 0x1000
#define SCICFG_RCLR 0x2000
#define SCICFG_BDR_OFFSET 2
#define SCICFG_TIRE_MASK 0x1c0
#define SCICFG_TIRE_OFFSET 6
#define SCICFG_RIRE_MASK 0xe00
#define SCICFG_RIRE_OFFSET 9

#define UTIM64MCFG_ENA 0x1

/* I/O FIFO buffer size */
#define SCI_FIFO_RX_SIZE (16 + 1)
#define SCI_FIFO_TX_SIZE (16 + 1)

/* DPS device struct */
typedef volatile struct _dps_utim64 {
  volatile uint32_t mcfg;
  volatile uint32_t mc[2];
  volatile uint32_t cc[4][2];
  volatile uint32_t cccfg[4];
} dps_utim64;

typedef volatile struct _dps_sci {
  volatile uint32_t txd;
  volatile uint32_t rxd;
  volatile uint32_t cfg;
} dps_sci;

/* DPS memory addr */
extern void *dps;

/* dps.c */
void dps_init(void);
void dps_close(void);
void dps_info(void);
void dps_utim64_read(Memory addr, Memory offset);
void dps_utim64_write(Memory addr, Memory offset);
bool dps_utim64_interrupt(void);
void dps_utim64_timer_sigalrm(int sig, siginfo_t *si, void *uc);
void dps_sci_rxd_read(Memory addr, Memory offset);
void dps_sci_txd_write(Memory addr, Memory offset);
void dps_sci_cfg_write(Memory addr, Memory offset);
bool dps_sci_recv(void);
bool dps_sci_interrupt(void);
void dps_lsflags_read(Memory addr, Memory offset);

#endif /* MIST32_DPS_H */
