#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#define DEBUG (0)

int gr[32];
unsigned int ip;
struct FLAGS flags;

union memory_p mem;
union memory_p sp;

void print_registers(void)
{
  unsigned int i;  
  
  printf("IP: 0x%08x\n", ip);
  printf("ZF: %d, PF: %d, CF: %d, OF: %d, SF %d\n",
	 flags.zero, flags.parity, flags.carry, flags.overflow, flags.sign);
  for(i = 0; i < 32; i++) {
    printf("GR%2d: 0x%08x (%11d) ", i, gr[i], gr[i]);
    if(!((i + 1) % 2)) { printf("\n"); }
  }
}

int exec(unsigned int offset, unsigned int size)
{
  Instruction *inst;
  OpcodeTable opcode_t;
  
  /* insert END instruction to tail */
  mem.word[size] = 0x1c600000;
  
  /* opcode table init */
  opcode_t = (OpcodeTable)opcode_table_init();
  
  ip = 0;
  
  do {
    /* instruction fetch */
    inst = (Instruction *)(mem.byte + ip);
    ip += 4;
    
    if(DEBUG) { printf("[debug] Ops: 0x%x, %x\n", inst->base.opcode, inst->value); }
    
    /* execute operation */
    (*(opcode_t[inst->base.opcode]))(inst);
    
    if(DEBUG) {
      print_registers();
      getchar();
    }
  } while(inst->base.opcode != 227);
  
  free(opcode_t);
  
  return 0;
}

int main(int argc, char **argv)
{
  unsigned int count;
  
  char *filename;
  FILE *fp;
  
  if(argc <= 1) {
    puts("error: no input files");
    return 0;
  }
  
  /* Allocate virtual memory */
  mem.byte = (unsigned char *)malloc(sizeof(unsigned char) * (1024 * 1024));
  if(mem.byte == NULL) {
    puts("error: can't allocate virtual memory.");
    return 1;
  }
  
  /* Read binary to execute */
  filename = argv[1];
  fp = fopen(filename, "rb");
  count = fread(mem.word, sizeof(Instruction), 512, fp);
  fclose(fp);
  
  /* set stack pointer (bottom of memory) */
  sp.byte = mem.byte + (1024 * 1024);
  
  /* Execute */
  exec(0, count);
  
  free(mem.byte);
  
  print_registers();
  
  return 0;
}
