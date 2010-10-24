#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#define BUFFER_SIZE 1024
#define DEBUG (0)

int gr[32];
unsigned int ip;
struct FLAGS flags;

union memory_p mem;
union memory_p sp;

void print_registers(void)
{
  unsigned int i;  
  
  printf("IP: 0x%08x\n", ip * 4);
  printf("ZF: %d, PF: %d, CF: %d, OF: %d, SF %d\n",
	 flags.zero, flags.parity, flags.carry, flags.overflow, flags.sign);
  for(i = 0; i < 32; i++) {
    printf("GR%2d: 0x%08x (%11d) ", i, gr[i], gr[i]);
    if(!((i + 1) % 2)) { printf("\n"); }
  }
}

int exec(Instruction *buffer, unsigned int size)
{
  Instruction *inst;
  OpcodeTable opcode_t;
  
  /* opcode table init */
  opcode_t = (OpcodeTable)opcode_table_init();
  
  ip = 0;
  
  while(ip < size) {
    /* instruction fetch */
    inst = buffer + ip++;
    
    if(DEBUG) { printf("[debug] Ops: 0x%x, %x\n", inst->base.opcode, inst->value); }
    
    /* execute operation */
    (*(opcode_t[inst->base.opcode]))(inst);
    
    if(DEBUG) { print_registers(); }
  }
  
  free(opcode_t);
  
  return 0;
}

int main(int argc, char **argv)
{
  Instruction *buffer;
  unsigned int count;
  
  char *filename;
  FILE *fp;
  
  if(argc <= 1) {
    puts("error: no input files");
    return 0;
  }
  
  /* Allocate buffer */
  buffer = (Instruction *)calloc(BUFFER_SIZE, sizeof(Instruction));
  if(buffer == NULL) {
    puts("error: can't allocate buffer.");
    return 1;
  }
  
  /* Read binary to execute */
  filename = argv[1];
  fp = fopen(filename, "rb");
  count = fread(buffer, sizeof(Instruction), BUFFER_SIZE, fp);
  fclose(fp);

  /* virtual memory allocate */
  mem.byte = (unsigned char *)malloc(sizeof(unsigned char) * (1024 * 1024));

  /* set stack pointer (bottom of memory) */
  sp.byte = mem.byte + (1024 * 1024);
  
  /* Execute */
  exec(buffer, count);
  
  free(mem.byte);
  free(buffer);
  
  print_registers();
  
  return 0;
}
