#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#define BUFFER_SIZE 1024

int ip, sp;
int flags;
int gr[32];

int exec(Instruction *buffer, unsigned int size)
{
  Instruction *inst;
  OpcodeTable *opcode_t;
  
  /* opcode table init */
  opcode_t = (OpcodeTable *)opcode_table_init();
  
  ip = 0;
  
  while(ip < size) {
    /* fetch */
    inst = buffer + ip++;
    (*(opcode_t[inst->base.opcode]))(inst);
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
  
  unsigned int i;
  
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

  /* Execute */
  exec(buffer, count);
  
  free(buffer);

  printf("IP: 0x%x, FLAGS: %d\n", ip * 4, flags);
  for(i = 0; i < 32; i++) {
    printf("GR%2d: 0x%08x ", i, gr[i]);
    if(!((i + 1) % 4)) { printf("\n"); }
  }
  
  return 0;
}
