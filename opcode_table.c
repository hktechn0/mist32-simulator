#include <stdlib.h>
#include "common.h"
#include "instructions.h"

OpcodeTable opcode_table_init(void)
{
  OpcodeTable opcode_t;
  opcode_t = (OpcodeTable)calloc(1024, sizeof(pOpcodeFunc));
  
  opcode_t[0] = i_nop;
  
  opcode_t[1] = i_add;
  opcode_t[2] = i_sub;
  opcode_t[3] = i_mul;
  opcode_t[4] = i_div;
  
  opcode_t[9] = i_sch;
  
  opcode_t[40] = i_lshl;
  opcode_t[41] = i_lshr;
  
  opcode_t[44] = i_lshl;
  opcode_t[45] = i_ashr;
  
  opcode_t[48] = i_rol;
  opcode_t[49] = i_ror;

  opcode_t[60] = i_and;
  opcode_t[61] = i_or;
  opcode_t[62] = i_not;
  opcode_t[63] = i_exor;
  opcode_t[64] = i_nand;
  opcode_t[65] = i_nor;

  opcode_t[70] = i_wb1;
  opcode_t[71] = i_wb2;
  opcode_t[72] = i_clb;
  opcode_t[73] = i_stb;
  opcode_t[74] = i_clw;
  opcode_t[75] = i_stw;
  
  opcode_t[90] = i_load;
  opcode_t[93] = i_store;
  
  opcode_t[137] = i_push;
  opcode_t[138] = i_pop;

  opcode_t[149] = i_mov;
  
  opcode_t[174] = i_halt;
  
  return opcode_t;
}
