#define DPS_SIZE 0x200
#define GCI_HUB_SIZE 0x400
#define GCI_HUB_HEADER_SIZE 0x100
#define GCI_NODE_SIZE 0x400

#define GCI_NODE_MAX 4
#define GCI_KMC_NUM 0
#define GCI_DISPLAY_NUM 1

/* DPS */
#define DPS_UTIM64A 0x000
#define DPS_UTIM64B 0x040

#define DPS_SCI 0x100
#define DPS_SCITXD 0x100
#define DPS_SCIRXD 0x104
#define DPS_SCICFG 0x108

#define DPS_MIMSR 0x120
#define DPS_LSFLAGS 0x1fc
#define DPS_LSFLAGS_SCIR 0x01

#define SCIRXD_VALID 0x80000000
#define SCICFG_TEN 0x1
#define SCICFG_REN 0x2
#define SCICFG_TCLR 0x1000
#define SCICFG_RCLR 0x2000
#define SCICFG_BDR_OFFSET 2
#define SCICFG_TIRE_OFFSET 6
#define SCICFG_RIRE_OFFSET 9

#define UTIM64MCFG_ENA 0x1

/* SCI FIFO file */
#define FIFO_SCI_TXD "./sci_txd"
#define FIFO_SCI_RXD "./sci_rxd"

#define GCI_KMC_PRIORITY 0x08
#define GCI_KMC_INT_PRIORITY 0xff
#define GCI_DISPLAY_PRIORITY 0xff
#define GCI_DISPLAY_INT_PRIORITY 0xff

#define GCI_KMC_AREA_SIZE 0x0004
#define GCI_DISPLAY_CHAR_SIZE 0xc000
#define GCI_DISPLAY_BITMAP_SIZE 0x400000
#define GCI_DISPLAY_AREA_SIZE (GCI_DISPLAY_CHAR_SIZE + GCI_DISPLAY_BITMAP_SIZE)

/* Display */
#define DISPLAY_CHAR_WIDTH 80
#define DISPLAY_CHAR_HEIGHT 34
#define DISPLAY_BITMAP_OFFSET 0xc000
#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 480

typedef volatile struct _gci_hub_info {
  unsigned int total;
  unsigned int space_size;
} gci_hub_info;

typedef volatile struct _gci_hub_node {
  unsigned int size;
  unsigned int priority;
  unsigned int _reserved1;
  unsigned int _reserved2;
  unsigned int _reserved3;
} gci_hub_node;

typedef volatile struct _gci_node_info {
  unsigned int area_size;
  unsigned int int_priority;
  volatile unsigned int int_factor;
  unsigned int _reserved;
} gci_node_info;

typedef struct _gci_node {
  gci_node_info *node_info;
  void *device_area;
} gci_node;

typedef volatile struct _dps_utim64 {
  volatile unsigned int mcfg;
  volatile unsigned int mc[2];
  volatile unsigned int cc0[2];
  volatile unsigned int cc1[2];
  volatile unsigned int cc2[2];
  volatile unsigned int cc3[2];
  volatile unsigned int cc0cfg;
  volatile unsigned int cc1cfg;
  volatile unsigned int cc2cfg;
  volatile unsigned int cc3cfg;
} dps_utim64;

typedef volatile struct _dps_sci {
  volatile unsigned int txd;
  volatile unsigned int rxd;
  volatile unsigned int cfg;
} dps_sci;

void io_init(void);
void io_close(void);
void *io_addr_get(Memory addr);
void io_load(Memory addr);
void io_store(Memory addr);
void io_info(void);
void gci_info(void);

void gci_kmc_read(Memory addr, Memory offset, void *mem);
void gci_display_write(Memory addr, Memory offset, void *mem);
