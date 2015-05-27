#ifndef MIST32_GCI_H
#define MIST32_GCI_H

#define GCI_HUB_SIZE 0x400
#define GCI_HUB_HEADER_SIZE 0x100
#define GCI_NODE_SIZE 0x400

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
#define KMC_FIFO_SCANCODE_SIZE 128

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

/* KMC FIFO */
#define KMC_SCANCODE_VALID 0x100
extern unsigned char fifo_scancode[KMC_FIFO_SCANCODE_SIZE];
extern unsigned int fifo_scancode_start, fifo_scancode_end;

/* GCI Hub / Node struct */
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

/* MMCC */
typedef volatile struct _gci_mmcc {
  volatile uint32_t init_command;
  volatile uint32_t sector_read;
  volatile uint32_t sector_write;
} gci_mmcc;

extern gci_hub_info *gci_hub;
extern gci_hub_node *gci_hub_nodes;
extern gci_node gci_nodes[4];

/* gci.c */
void gci_init(void);
void gci_close(void);
void gci_info(void);
void gci_kmc_read(Memory addr, Memory offset, void *mem);
bool gci_kmc_interrupt(void);
void gci_display_write(Memory addr, Memory offset, void *mem);
void gci_mmcc_read(Memory addr, Memory offset, void *mem);
void gci_mmcc_write(Memory addr, Memory offset, void *mem);

#endif /* MIST32_GCI_H */
