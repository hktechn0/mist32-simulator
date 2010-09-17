#include "common.h"

unsigned int immediate_i11(Instruction inst) {
  return (inst.i11.immediate2 << 10) + inst.i11.immediate1;
}

unsigned int immediate_i16(Instruction inst) {
  return (inst.i16.immediate2 << 10) + inst.i16.immediate1;
}
