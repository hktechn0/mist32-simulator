struct instruction_base {
  unsigned int payload : 21;
  unsigned int opcode  : 10;
  unsigned int _extend : 1;
};

struct instruction_i11 {
  unsigned int immediate2   : 5;
  unsigned int operand      : 5;
  unsigned int immediate1   : 6;
  unsigned int condition    : 4;
  unsigned int is_immediate : 1;
  unsigned int opcode       : 10;
  unsigned int _extend      : 1;
};

struct instruction_i16 {
  unsigned int immediate2   : 5;
  unsigned int operand      : 5;
  unsigned int immediate1   : 11;
  unsigned int opcode       : 10;
  unsigned int _extend      : 1;
};

struct instruction_o2 {
  unsigned int operand2     : 5;
  unsigned int operand1     : 5;
  unsigned int _reserve     : 6;
  unsigned int condition    : 4;
  unsigned int is_immediate : 1;
  unsigned int opcode       : 10;
  unsigned int _extend      : 1;
};

struct instruction_o1 {
  unsigned int ___reserve   : 5;
  unsigned int operand1     : 5;
  unsigned int _reserve     : 6;
  unsigned int condition    : 4;
  unsigned int is_immediate : 1;
  unsigned int opcode       : 10;
  unsigned int _extend      : 1;
};

struct instruction_c {
  unsigned int immediate    : 16;
  unsigned int condition    : 4;
  unsigned int is_immediate : 1;
  unsigned int opcode       : 10;
  unsigned int _extend      : 1;
};

struct instruction_ji16 {
  unsigned int immediate    : 16;
  unsigned int condition    : 4;
  unsigned int is_immediate : 1;
  unsigned int opcode       : 10;
  unsigned int _extend      : 1;
};

struct instruction_jo1 {
  unsigned int ___reserve   : 5;
  unsigned int operand1     : 5;
  unsigned int _reserve     : 6;
  unsigned int condition    : 4;
  unsigned int is_immediate : 1;
  unsigned int opcode       : 10;
  unsigned int _extend      : 1;
};

typedef union _instruction {
  unsigned int value;
  struct instruction_base base;
  struct instruction_i11  i11;
  struct instruction_i16  i16;
  struct instruction_o2   o2;
  struct instruction_o1   o1;
  struct instruction_c    c;
  struct instruction_ji16 ji16;
  struct instruction_jo1  jo1;
} Instruction;
