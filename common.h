#include <stdbool.h>
#include <stdint.h>

#define NO_DEBUG 0

#define DEBUG_TRUE (1 && !NO_DEBUG)
#define DEBUG_FALSE 0

#define DEBUG_REG     DEBUG_TRUE
#define DEBUG_TRACE   DEBUG_FALSE
#define DEBUG_STACK   DEBUG_FALSE
#define DEBUG_MON     DEBUG_TRUE
#define DEBUG_IO      DEBUG_TRUE
#define DEBUG_DPS     DEBUG_FALSE
#define DEBUG_EXIT_B0 DEBUG_FALSE

#if !NO_DEBUG
#define DPUTS if(DEBUG || step_by_step) printf
#define DEBUGLD if(DEBUG_LD || step_by_step) printf
#define DEBUGST if(DEBUG_ST || step_by_step) printf
#define DEBUGLDHW if(DEBUG_HW) printf
#define DEBUGSTHW if(DEBUG_HW) printf
#define DEBUGLDPHY if(DEBUG_PHY) printf
#define DEBUGSTPHY if(DEBUG_PHY) printf
#define DEBUGJMP if(DEBUG_JMP || step_by_step) printf
#define DEBUGMON if(DEBUG_MON || step_by_step) printf
#define DEBUGINT if(DEBUG_INT || step_by_step) printf
#define DEBUGIO if(DEBUG_IO || step_by_step) printf
#define DEBUGMMU if(DEBUG_MMU || step_by_step) printf
#else
#define DPUTS if(0) printf
#define DEBUGLD if(0) printf
#define DEBUGST if(0) printf
#define DEBUGLDHW if(0) printf
#define DEBUGSTHW if(0) printf
#define DEBUGLDPHY if(0) printf
#define DEBUGSTPHY if(0) printf
#define DEBUGJMP if(0) printf
#define DEBUGMON if(0) printf
#define DEBUGINT if(0) printf
#define DEBUGIO if(0) printf
#define DEBUGMMU if(0) printf
#endif

#define TRACEBACK_MAX 1024
#define MONITOR_RECV_INTERVAL_MASK (0x1000 - 1)

/* Memory Size */
#define MEMORY_MAX_ADDR 0x08000000

#define EM_MIST32 0x1032
#define STACK_DEFAULT MEMORY_MAX_ADDR

#define NOP_INSN (0x100 << 21)

#define msb(word) ((word) >> 31)
#define msb64(word) ((word) >> 63)
#define NOT(reg) (reg = ~reg)
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define SIGN_EXT6(n) ((n & 0x20) ? (n | 0xffffffc0) : (n & 0x3f))
#define SIGN_EXT8(n) ((n & 0x80) ? (n | 0xffffff00) : (n & 0xff))
#define SIGN_EXT16(n) ((n & 0x8000) ? (n | 0xffff0000) : (n & 0xffff))

#define mem_barrier() asm volatile("" ::: "memory")

/* Register constants */
#define GR_TMP 7
#define GR_GLOBL 29
#define GR_BASE 30
#define GR_RET 31

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

/* Instruction format */
#include "insn_format.h"

typedef uint32_t Memory;

/* Debug flags */
extern bool DEBUG, DEBUG_LD, DEBUG_ST, DEBUG_JMP, DEBUG_HW, DEBUG_PHY, DEBUG_INT, DEBUG_MMU;
extern bool MONITOR;
extern bool step_by_step, exec_finish;

/* Break points */
extern Memory breakp[100];
extern unsigned int breakp_next;

/* Traceback */
extern Memory traceback[1024];
extern unsigned int traceback_next;

/* Vritual Memory */
extern Memory vmem;

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

/* utils */
void print_instruction(Instruction insn);
void print_registers(void);
void print_stack(Memory sp);
void print_traceback(void);
void abort_sim(void);
void step_by_step_pause(void);

/* simulator */
int exec(Memory entry);

/* FIXME: include */
#include "memory.h"
#include "io.h"
