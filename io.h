#ifndef MIST32_IO_H
#define MIST32_IO_H

/* FIFO remaining */
#define FIFO_USED(start, end, size) ((end + size - start) % size)

/* io.c */
void io_init(void);
void io_close(void);
void *io_addr_get(Memory addr);
void io_load(Memory addr);
void io_store(Memory addr);

#endif /* MIST32_IO_H */
