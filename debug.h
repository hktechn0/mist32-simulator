#ifndef MIST32_DEBUG_H
#define MIST32_DEBUG_H

#define NO_DEBUG 0

#define DEBUG_TRUE (1 && !NO_DEBUG && !QUIET_MODE)
#define DEBUG_FALSE 0

#define DEBUG_REG     DEBUG_TRUE
#define DEBUG_TRACE   DEBUG_FALSE
#define DEBUG_STACK   DEBUG_FALSE
#define DEBUG_MON     DEBUG_TRUE
#define DEBUG_IO      DEBUG_TRUE
#define DEBUG_DPS     DEBUG_FALSE
#define DEBUG_EXIT_B0 DEBUG_FALSE

#if !NO_DEBUG
#define DEBUG_MEM (DEBUG_LD || DEBUG_ST || DEBUG_HW || DEBUG_PHY)
#else
#define DEBUG_MEM 0
#endif

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

#define NOTICE if(!QUIET_MODE) printf

/* Debug flags */
extern bool DEBUG, DEBUG_LD, DEBUG_ST, DEBUG_JMP, DEBUG_HW, DEBUG_PHY, DEBUG_INT, DEBUG_MMU;
extern bool MONITOR, TESTSUITE_MODE, QUIET_MODE, SCI_USE_STDIN, SCI_USE_STDOUT;
extern bool step_by_step;

/* utils.c */
void debug_load_hw(Memory addr, unsigned int data);
void debug_store_hw(Memory addr, unsigned int data);
void debug_load8(Memory addr, unsigned char data);
void debug_load16(Memory addr, unsigned short data);
void debug_load32(Memory addr, unsigned int data);
void debug_store8(Memory addr, unsigned char data);
void debug_store16(Memory addr, unsigned short data);
void debug_store32(Memory addr, unsigned int data);
void debug_push(Memory addr, unsigned int data);
void debug_pop(Memory addr, unsigned int data);

#endif /* MIST32_DEBUG_H */
