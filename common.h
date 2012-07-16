#include "memory.h"
#include "instruction_format.h"

#define EM_MIST32 0x1032

#define DPUTS if(DEBUG) printf
#define DEBUG (1)

extern Memory vmem;

extern int gr[32];
extern Memory sp;
extern Memory pc;

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
unsigned int immediate_i11(Instruction *inst);
unsigned int immediate_i16(Instruction *inst);
void ops_o2_i11(Instruction *inst, int **op1, int *op2);
int src_o2_i11(Instruction *inst);
int src_o1_i11(Instruction *inst);
int check_condition(Instruction *inst);
void clr_flags(void);
void set_flags(int value);

/* simulator */
void print_registers(void);
int exec(Memory entry);
