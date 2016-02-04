#ifndef MIST32_COMMON_H
#define MIST32_COMMON_H

#include <stdbool.h>
#include <stdint.h>

/* Hardware Clock: 50MHz */
#define HARDWARE_CLOCK_HZ 50000000

/* ELF magic */
#define EM_MIST32 0x1032

/* Memory Size */
#define MEMORY_MAX_ADDR 0x08000000

/* default stack pointer */
#define STACK_DEFAULT MEMORY_MAX_ADDR

/* Default filenames */
#define SOCKET_SCI_DEFAULT "/tmp/sci.sock"
#define FIFO_DISPLAY_CHAR_DEFAULT "./gci_display_char"

/* SCI socket / Display FIFO filename from options */
extern char *sci_sock_file;
extern char *gci_mmcc_image_file;

/* Termination flags */
extern bool exec_finish;
extern int return_code;

typedef uint32_t Memory;

/* Break points */
extern Memory breakp[100];
extern unsigned int breakp_next;

/* Traceback */
#define TRACEBACK_MAX 1024
extern Memory traceback[TRACEBACK_MAX];
extern unsigned int traceback_next;

/* simulator.c */
int exec(Memory entry);

#endif /* MIST32_COMMON_H */
