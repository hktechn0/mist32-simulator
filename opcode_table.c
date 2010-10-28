#include <stdlib.h>
#include "common.h"
#include "instructions.h"

OpcodeTable opcode_table_init(void)
{
  OpcodeTable opcode_t;
  opcode_t = (OpcodeTable)calloc(1024, sizeof(pOpcodeFunc));
  
  opcode_t[0] = i_add;
  opcode_t[1] = i_sub;
  opcode_t[2] = i_mull;
  opcode_t[3] = i_mulh;
  opcode_t[4] = i_div;

  /*opcode_t[6] = i_cmp;*/
  
  opcode_t[9] = i_sch;
  
  opcode_t[64] = i_lshl;
  opcode_t[65] = i_lshr;  
  opcode_t[69] = i_ashr;
  opcode_t[72] = i_rol;
  opcode_t[73] = i_ror;

  opcode_t[96] = i_and;
  opcode_t[97] = i_or;
  opcode_t[98] = i_not;
  opcode_t[99] = i_exor;
  opcode_t[100] = i_nand;
  opcode_t[101] = i_nor;

  opcode_t[106] = i_wb1;
  opcode_t[107] = i_wb2;
  opcode_t[108] = i_clb;
  opcode_t[109] = i_stb;
  opcode_t[110] = i_clw;
  opcode_t[111] = i_stw;
  
  opcode_t[128] = i_load;
  opcode_t[131] = i_store;
  
  opcode_t[136] = i_push;
  /*opcode_t[137] = i_ppush;*/
  opcode_t[144] = i_pop;

  /*opcode_t[160] = i_pjmp;
    opcode_t[162] = i_djmp;*/

  opcode_t[224] = i_nop;
  opcode_t[225] = i_halt;
  opcode_t[226] = i_mov;
  
  return opcode_t;
}
