#ifndef MIST32_REGISTERS_H
#define MIST32_REGISTERS_H

#include "debug.h"

/* Register constants */
#define GR_TMP 7
#define GR_GLOBL 29
#define GR_BASE 30
#define GR_RET 31

/* PSR Flags */
#define PSR_MMUMOD_MASK 0x3
#define PSR_MMUMOD (PSR & PSR_MMUMOD_MASK)
#define PSR_MMUMOD_DIRECT 0x0
#define PSR_MMUMOD_L1 0x1
#define PSR_MMUMOD_L2 0x2
#define PSR_IM_ENABLE 0x4
#define PSR_CMOD_MASK 0x60
#define PSR_CMOD_KERNEL 0x00
#define PSR_CMOD_USER 0x60
#define PSR_MMUPS_MASK 0x380
#define PSR_MMUPS ((PSR & PSR_MMUPS_MASK) >> 7)
#define PSR_MMUPS_4KB 0x1

/* Conditional FLAGS */
typedef union {
  struct {
    unsigned int _invalid    : 1;
    unsigned int _reserved : 26;
    unsigned int sign      : 1;
    unsigned int overflow  : 1;
    unsigned int carry     : 1;
    unsigned int parity    : 1;
    unsigned int zero      : 1;
  };
  uint32_t flags;
} FLAGS;

/* General Register */
extern int32_t GR[32];

/* System Register */
extern FLAGS FLAGR;
extern Memory PCR, next_PCR;
extern Memory SPR, KSPR, USPR;
extern uint32_t PSR;
extern Memory IOSR;
extern Memory PDTR, KPDTR;
extern Memory IDTR;
extern uint32_t TIDR;
extern uint64_t FRCR;
extern uint32_t FI0R, FI1R;

#if !NO_DEBUG
extern FLAGS prev_FLAGR;
#endif

/* Previous System Registers */
extern FLAGS PFLAGR;
extern Memory PPCR;
extern uint32_t PPSR;
extern Memory PPDTR;
extern uint32_t PTIDR;

#endif /* MIST32_REGISTERS_H */
