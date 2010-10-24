void i_nop(Instruction *inst);

void i_add(Instruction *inst);
void i_sub(Instruction *inst);
void i_mul(Instruction *inst);
void i_div(Instruction *inst);

void i_sch(Instruction *inst);

void i_lshl(Instruction *inst);
void i_lshr(Instruction *inst);
void i_ashr(Instruction *inst);
void i_ror(Instruction *inst);

void i_and(Instruction *inst);
void i_or(Instruction *inst);
void i_not(Instruction *inst);
void i_exor(Instruction *inst);
void i_nand(Instruction *inst);
void i_nor(Instruction *inst);

void i_load(Instruction *inst);
void i_store(Instruction *inst);

void i_push(Instruction *inst);
void i_pop(Instruction *inst);

void i_mov(Instruction *inst);

void i_halt(Instruction *inst);
