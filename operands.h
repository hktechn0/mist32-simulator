/* fetch immediate for i11 format */
static inline unsigned int immediate_ui11(Instruction insn)
{
  return (insn.i11.immediate1 << 5) + insn.i11.immediate2;
}

static inline int immediate_i11(Instruction insn)
{
  int imm;
  
  /* sign extend */
  imm = ((int)immediate_ui11(insn) << 21) >> 21;
  
  return imm;
}

/* fetch immediate for i16 format */
static inline unsigned int immediate_ui16(Instruction insn)
{
  return (insn.i16.immediate1 << 5) + insn.i16.immediate2;
}

static inline int immediate_i16(Instruction insn)
{
  int imm;

  /* sign extend */
  imm = ((int)immediate_ui16(insn) << 16) >> 16;
  
  return imm;
}

/* fetch o2 or i11 format operand
  insn: Instruction struct,
  op1:  store operand1 pointer (writable, directly to register),
  op2:  store operand2 (read only) */
static inline void ops_o2_i11(Instruction insn, int **op1, int *op2)
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

static inline void ops_o2_ui11(Instruction insn, unsigned int **op1, unsigned int *op2)
{
  if(insn.i11.is_immediate) {
    *op1 = (unsigned int *)&(GR[insn.i11.operand]);
    *op2 = immediate_ui11(insn);
  }
  else {
    *op1 = (unsigned int *)&(GR[insn.o2.operand1]);
    *op2 = (unsigned int)GR[insn.o2.operand2];
  }
}

/* return source operand value (I11, O2 format) */
static inline int src_o2_i11(Instruction insn)
{
  if(insn.i11.is_immediate) {
    return immediate_i11(insn);
  }
  else {
    return GR[insn.o2.operand2];
  }
}

/* return source operand value (I11, O1 format) */
static inline int src_o1_i11(Instruction insn)
{
  if(insn.i11.is_immediate) {
    return immediate_i11(insn);
  }
  else {
    return GR[insn.o1.operand1];
  }
}

/* return source operand value (JI16, JO1 format) */
static inline int src_jo1_ji16(Instruction insn)
{
  if(insn.ji16.is_immediate) {
    return ((int)insn.ji16.immediate << 16) >> 14;
  }
  else {
    return GR[insn.jo1.operand1];
  }
}

/* return source operand value (JI16, JO1 format) */
static inline unsigned int src_jo1_jui16(Instruction insn)
{
  if(insn.ji16.is_immediate) {
    /* no sign extend */
    return insn.ji16.immediate << 2;
  }
  else {
    return GR[insn.jo1.operand1];
  }
}

/* Check condition code and flags */
/* return: true, false */
static inline int check_condition(Instruction insn)
{
  switch(insn.ji16.condition) {
  case 0:
    return 1;
    break;
  case 1:
    if(FLAGR.zero) { return 1; }
    else { return 0; }
    break;
  case 2:
    if(!FLAGR.zero) { return 1; }
    else { return 0; }
    break;
  case 3:
    if(FLAGR.sign) { return 1; }
    else { return 0; }
    break;
  case 4:
    if(!FLAGR.sign) { return 1; }
    else { return 0; }
    break;
  case 5:
    if(FLAGR.parity) { return 1; }
    else { return 0; }
    break;
  case 6:
    if(!FLAGR.parity) { return 1; }
    else { return 0; }
    break;
  case 7:
    if(FLAGR.overflow) { return 1; }
    else { return 0; }
    break;
  case 8:
    if(FLAGR.carry) { return 1; }
    else { return 0; }
    break;
  case 9:
    if(!FLAGR.carry) { return 1; }
    else { return 0; }
    break;
  case 0xA:
    if(FLAGR.carry && !FLAGR.zero) { return 1; }
    else { return 0; }
    break;
  case 0xB:
    if(!FLAGR.carry || FLAGR.zero) { return 1; }
    else { return 0; }
    break;
  case 0xC:
    if(FLAGR.sign == FLAGR.overflow) { return 1; }
    else { return 0; }
    break;
  case 0xD:
    if(FLAGR.sign != FLAGR.overflow) { return 1; }
    else { return 0; }
    break;
  case 0xE:
    if(!((FLAGR.sign ^ FLAGR.overflow) || FLAGR.zero)) { return 1; }
    else { return 0; }
    break;
  case 0xF:
    if((FLAGR.sign ^ FLAGR.overflow) || FLAGR.zero) { return 1; }
    else { return 0; }
    break;
  default:
    break;
  }
  
  return 0;  
}
