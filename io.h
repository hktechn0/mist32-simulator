#include <signal.h>

#define DPS_SIZE 0x200
#define GCI_HUB_SIZE 0x400
#define GCI_HUB_HEADER_SIZE 0x100
#define GCI_NODE_SIZE 0x400

/* DPS */
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

/* GCI */
#define GCI_NODE_MAX 4
#define GCI_KMC_NUM 0
#define GCI_DISPLAY_NUM 1
#define GCI_MMCC_NUM 2

/* Display */
#define DISPLAY_CHAR_WIDTH 80
#define DISPLAY_CHAR_HEIGHT 34
#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 480
#define DISPLAY_BITMAP_OFFSET 0xc000

/* KMC */
#define KMC_SCANCODE_VALID 0x100

/* MMCC */
#define MMCC_BUFFER_OFFSET 0x040
#define MMCC_BUFFER_SIZE 0x200
#define MMCC_SECTOR_SIZE 512

#define GCI_MMCC_INIT_COMMAND 0x000
#define GCI_MMCC_SECTOR_READ 0x004
#define GCI_MMCC_SECTOR_WRITE 0x008

/* Priority */
#define GCI_KMC_PRIORITY 0x08
#define GCI_KMC_INT_PRIORITY 0xff
#define GCI_DISPLAY_PRIORITY 0xff
#define GCI_DISPLAY_INT_PRIORITY 0xff
#define GCI_MMCC_PRIORITY 0xff
#define GCI_MMCC_INT_PRIORITY 0xff

/* Area Size */
#define GCI_KMC_AREA_SIZE 0x0004
#define GCI_DISPLAY_CHAR_SIZE 0x0000c000
#define GCI_DISPLAY_BITMAP_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT * 4)
#define GCI_DISPLAY_AREA_SIZE (GCI_DISPLAY_CHAR_SIZE + GCI_DISPLAY_BITMAP_SIZE)
#define GCI_MMCC_AREA_SIZE 0x0240

/* I/O FIFO Buffer */
#define FIFO_USED(start, end, size) ((end + size - start) % size)
#define SCI_FIFO_RX_SIZE (16 + 1)
#define SCI_FIFO_TX_SIZE (16 + 1)
#define KMC_FIFO_SCANCODE_SIZE 128

/* SCI Scoket / Display FIFO */
#define SOCKET_SCI "./sci.sock"
#define FIFO_DISPLAY_CHAR "./gci_display_char"

typedef volatile struct _gci_hub_info {
  uint32_t total;
  uint32_t space_size;
} gci_hub_info;

typedef volatile struct _gci_hub_node {
  uint32_t size;
  uint32_t priority;
  uint32_t _reserved1;
  uint32_t _reserved2;
  uint32_t _reserved3;
  uint32_t _reserved4;
  uint32_t _reserved5;
  uint32_t _reserved6;
} gci_hub_node;

typedef volatile struct _gci_node_info {
  uint32_t area_size;
  uint32_t int_priority;
  volatile uint32_t int_factor;
  uint32_t _reserved;
} gci_node_info;

typedef struct _gci_node {
  gci_node_info *node_info;
  void *device_area;
  bool int_dispatch;
  bool int_issued;
} gci_node;

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

/* MMCC */
typedef volatile struct _gci_mmcc {
  volatile uint32_t init_command;
  volatile uint32_t sector_read;
  volatile uint32_t sector_write;
} gci_mmcc;

extern void *dps;
extern char *sci_sock_file;

extern gci_hub_info *gci_hub;
extern gci_hub_node *gci_hub_nodes;
extern gci_node gci_nodes[4];
extern char *gci_mmcc_image;

extern unsigned char fifo_scancode[KMC_FIFO_SCANCODE_SIZE];
extern unsigned int fifo_scancode_start, fifo_scancode_end;

/* io.c */
void io_init(void);
void io_close(void);
void *io_addr_get(Memory addr);
void io_load(Memory addr);
void io_store(Memory addr);

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

/* gci.c */
void gci_init(void);
void gci_close(void);
void gci_info(void);
void gci_kmc_read(Memory addr, Memory offset, void *mem);
bool gci_kmc_interrupt(void);
void gci_display_write(Memory addr, Memory offset, void *mem);
void gci_mmcc_read(Memory addr, Memory offset, void *mem);
void gci_mmcc_write(Memory addr, Memory offset, void *mem);
