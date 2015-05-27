#ifndef MIST32_UTILS_H
#define MIST32_UTILS_H

#include "insn_format.h"

#define msb(word) ((word) >> 31)
#define msb64(word) ((word) >> 63)
#define NOT(reg) (reg = ~reg)
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define SIGN_EXT6(n) ((n & 0x20) ? (n | 0xffffffc0) : (n & 0x3f))
#define SIGN_EXT8(n) ((n & 0x80) ? (n | 0xffffff00) : (n & 0xff))
#define SIGN_EXT16(n) ((n & 0x8000) ? (n | 0xffff0000) : (n & 0xffff))

#define mem_barrier() { asm volatile("" ::: "memory"); }

/* utils.c */
void print_instruction(Instruction insn);
void print_registers(void);
void print_stack(Memory sp);
void print_traceback(void);
void abort_sim(void);
void step_by_step_pause(void);

#endif /* MIST32_UTILS_H */
