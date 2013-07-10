#define KMC_FIFO_SCANCODE_SIZE 128

extern char fifo_scancode[KMC_FIFO_SCANCODE_SIZE];
extern unsigned int fifo_scancode_start, fifo_scancode_end;

extern gci_node gci_nodes[4];
extern int fd_dispchar;

void gci_kmc_read(Memory addr, Memory offset, void *mem);
bool gci_kmc_interrupt(void);

void gci_display_write(Memory addr, Memory offset, void *mem);
