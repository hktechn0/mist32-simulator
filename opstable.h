#include "instructions.h"

#define OPCODE_MAX 1024

static const pOpcodeFunc opcode_t[OPCODE_MAX] __attribute__ ((aligned(64))) = {
  [0] = i_add,
  [1] = i_sub,
  [2] = i_mull,
  [3] = i_mulh,
  [4] = i_udiv,
  [5] = i_umod,
  [6] = i_cmp,
  [7] = i_div,
  [8] = i_mod,
  [9] = i_neg,

  [14] = i_addc,
  [16] = i_inc,
  [17] = i_dec,
  [28] = i_sext8,
  [29] = i_sext16,
  
  [64] = i_shl,
  [65] = i_shr,
  [69] = i_sar,
  [72] = i_rol,
  [73] = i_ror,

  [96] = i_and,
  [97] = i_or,
  [98] = i_xor,
  [99] = i_not,
  [100] = i_nand,
  [101] = i_nor,
  [102] = i_xnor,
  [103] = i_test,

  [106] = i_wl16,
  [107] = i_wh16,
  [108] = i_clrb,
  [109] = i_setb,
  [110] = i_clr,
  [111] = i_set,
  [112] = i_revb,
  [113] = i_rev8,
  [114] = i_getb,
  [115] = i_get8,
  
  [118] = i_lil,
  [119] = i_lih,
  [122] = i_ulil,

  [128] = i_ld8,
  [129] = i_ld16,
  [130] = i_ld32,
  [131] = i_st8,
  [132] = i_st16,
  [133] = i_st32,
  
  [136] = i_push,
  [137] = i_pushpc,
  [144] = i_pop,

  [154] = i_ld8,  /* ldd8  */
  [155] = i_ld16, /* ldd16 */
  [156] = i_ld32, /* ldd32 */
  [157] = i_st8,  /* std8  */
  [158] = i_st16, /* std16 */
  [159] = i_st32, /* std32 */

  [160] = i_bur,
  [161] = i_br,
  [162] = i_b,
  [163] = i_ib,
  
  [176] = i_bur, /* burn */
  [177] = i_br,  /* brn  */
  [178] = i_b,   /* bn   */

  [192] = i_srspr,
  [193] = i_srpdtr,
  [194] = i_srpidr,
  [195] = i_srcidr,
  [196] = i_srmoder,
  [197] = i_srieir,
  [201] = i_srkpdtr,
  [202] = i_srmmur,
  [203] = i_sriosr,
  [204] = i_srtidr,
  [205] = i_srppsr,
  [206] = i_srppcr,
  [207] = i_sruspr,
  [208] = i_srppdtr,
  [209] = i_srptidr,
  [211] = i_srpsr,
  [212] = i_srfrcr,
  [213] = i_srfrclr,
  [214] = i_srfrchr,
  [215] = i_srpflagr,
  [216] = i_srfi0r,
  [217] = i_srfi1r,

  [224] = i_srspw,
  [225] = i_srpdtw,
  [229] = i_srieiw,
  [233] = i_srkpdtw,
  [234] = i_srmmuw,
  [237] = i_srppsw,
  [238] = i_srppcw,
  [239] = i_sruspw,
  [240] = i_srppdtw,
  [241] = i_srptidw,
  [242] = i_sridtw,
  [243] = i_srpsw,
  [247] = i_srpflagw,
  [255] = i_srspadd,

  [256] = i_nop,
  [257] = i_halt,
  [258] = i_move,
  [259] = i_movepc,

  [288] = i_swi,
  [289] = i_tas,
  [290] = i_idts,
};
