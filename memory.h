#ifndef MIST32_MEMORY_H
#define MIST32_MEMORY_H

#include "common.h"

extern int memory_is_fault;
extern Memory memory_io_writeback;

/* memory.c */
void memory_init(void);
void memory_free(void);

#endif /* MIST32_MEMORY_H */
