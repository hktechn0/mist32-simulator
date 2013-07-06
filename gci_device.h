extern gci_node gci_nodes[4];
extern int fd_dispchar;

void gci_kmc_read(Memory addr, Memory offset, void *mem);
void gci_display_write(Memory addr, Memory offset, void *mem);
