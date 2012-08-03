#include <stdbool.h>

#include "memory.h"
#include "instruction_format.h"

#define DPUTS if(DEBUG) printf

#define EM_MIST32 0x1032
#define IO_START_ADDR 0xffec75fc
#define STACK_DEFAULT 0x04000000
#define OPCODE_MAX 1024

#define msb(word) (!!((word) & 0x80000000))
#define NOT(reg) (reg = ~reg)

extern bool DEBUG;
extern bool DEBUG_I;

/* Vritual Memory */
extern Memory vmem;

/* General Register */
extern int gr[32];

/* System Register */
extern Memory sp;
extern Memory pc;
extern Memory next_pc;

extern unsigned int sr1;
extern Memory idtr;
extern unsigned long long frcr;

struct FLAGS {
  unsigned int          : 27;
  unsigned int sign     : 1;
  unsigned int overflow : 1;
  unsigned int carry    : 1;
  unsigned int parity   : 1;
  unsigned int zero     : 1;
};
extern struct FLAGS flags;

/* Function pointer void *pOpcodeFunc(Instruction *) */
typedef void (*pOpcodeFunc) (Instruction *);
typedef pOpcodeFunc* OpcodeTable;

/* opcode */
OpcodeTable opcode_table_init(void);

/* utils */
unsigned int immediate_ui11(Instruction *inst);
int immediate_i11(Instruction *inst);
unsigned int immediate_ui16(Instruction *inst);
int immediate_i16(Instruction *inst);
void ops_o2_i11(Instruction *inst, int **op1, int *op2);
void ops_o2_ui11(Instruction *inst, unsigned int **op1, unsigned int *op2);
int src_o2_i11(Instruction *inst);
int src_o1_i11(Instruction *inst);
int src_jo1_ji16(Instruction *inst);
unsigned int src_jo1_jui16(Instruction *inst);
int check_condition(Instruction *inst);
void clr_flags(void);
void set_flags(int value);
void set_flags_add(unsigned int result, unsigned int dest, unsigned int src);
void set_flags_sub(unsigned int result, unsigned int dest, unsigned int src);
void print_registers(void);
void print_stack(Memory sp);

/* simulator */
int exec(Memory entry);
