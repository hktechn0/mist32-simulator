/* fetch immediate for i11 format */
static inline uint32_t immediate_ui11(Instruction insn)
{
  return (insn.i11.immediate1 << 5) + insn.i11.immediate2;
}

static inline int32_t immediate_i11(Instruction insn)
{
  /* sign extend */
  return ((int32_t)immediate_ui11(insn) << 21) >> 21;
}

/* fetch immediate for i16 format */
static inline uint32_t immediate_ui16(Instruction insn)
{
  return (insn.i16.immediate1 << 5) + insn.i16.immediate2;
}

static inline int32_t immediate_i16(Instruction insn)
{
  /* sign extend */
  return ((int32_t)immediate_ui16(insn) << 16) >> 16;
}

/* fetch o2 or i11 format operand
  insn: Instruction struct,
  op1:  store operand1 pointer (writable, directly to register),
  op2:  store operand2 (read only) */
static inline void ops_o2_i11(Instruction insn, int32_t **op1, int32_t *op2)
{
  if(insn.i11.is_immediate) {
    *op1 = &(GR[insn.i11.operand]);
    *op2 = immediate_i11(insn);
  }
  else {
    *op1 = &(GR[insn.o2.operand1]);
    *op2 = GR[insn.o2.operand2];
  }
}

static inline void ops_o2_ui11(Instruction insn, uint32_t **op1, uint32_t *op2)
{
  if(insn.i11.is_immediate) {
    *op1 = (uint32_t *)&(GR[insn.i11.operand]);
    *op2 = immediate_ui11(insn);
  }
  else {
    *op1 = (uint32_t *)&(GR[insn.o2.operand1]);
    *op2 = (uint32_t)GR[insn.o2.operand2];
  }
}

/* return source operand value (I11, O2 format) */
static inline int32_t src_o2_i11(Instruction insn)
{
  if(insn.i11.is_immediate) {
    return immediate_i11(insn);
  }
  else {
    return GR[insn.o2.operand2];
  }
}

/* return source operand value (I11, O1 format) */
static inline int32_t src_o1_i11(Instruction insn)
{
  if(insn.i11.is_immediate) {
    return immediate_i11(insn);
  }
  else {
    return GR[insn.o1.operand1];
  }
}

/* return source operand value (JI16, JO1 format) */
static inline int32_t src_jo1_ji16(Instruction insn)
{
  if(insn.ji16.is_immediate) {
    return ((int)insn.ji16.immediate << 16) >> 14;
  }
  else {
    return GR[insn.jo1.operand1];
  }
}

/* return source operand value (JI16, JO1 format) */
static inline uint32_t src_jo1_jui16(Instruction insn)
{
  if(insn.ji16.is_immediate) {
    /* no sign extend */
    return insn.ji16.immediate << 2;
  }
  else {
    return (uint32_t)GR[insn.jo1.operand1];
  }
}

/* Check condition code and flags */
/* return: true, false */
static inline bool check_condition(Instruction insn)
{
  switch(insn.ji16.condition) {
  case 0:
    return true;
    break;
  case 1:
    return (FLAGR.zero);
    break;
  case 2:
    return (!FLAGR.zero);
    break;
  case 3:
    return (FLAGR.sign);
    break;
  case 4:
    return (!FLAGR.sign);
    break;
  case 5:
    return (FLAGR.parity);
    break;
  case 6:
    return (!FLAGR.parity);
    break;
  case 7:
    return (FLAGR.overflow);
    break;
  case 8:
    return (FLAGR.carry);
    break;
  case 9:
    return (!FLAGR.carry);
    break;
  case 0xA:
    return (FLAGR.carry && !FLAGR.zero);
    break;
  case 0xB:
    return (!FLAGR.carry || FLAGR.zero);
    break;
  case 0xC:
    return (FLAGR.sign == FLAGR.overflow);
    break;
  case 0xD:
    return (FLAGR.sign != FLAGR.overflow);
    break;
  case 0xE:
    return (!((FLAGR.sign ^ FLAGR.overflow) || FLAGR.zero));
    break;
  case 0xF:
    return ((FLAGR.sign ^ FLAGR.overflow) || FLAGR.zero);
    break;
  default:
    break;
  }
  
  return false;
}
