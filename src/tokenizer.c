#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tokenizer.h"
#include "instruction-types.h"

// Don't use with an empty string please
splitInstr tokenize_line(char *line_in) {
  char operands[MAX_OPERANDS][MAX_OPERAND_LENGTH];
  char opcode[MAX_OPCODE_LENGTH];

  // the bit of the string still to be tokenised
  char *leftover = malloc(sizeof(char*));
  // get the opcode
  char *cur_token = strtok_r(line_in, " ", &leftover);
  strcpy(opcode, cur_token);

  // this shouldn't be NULL because we're expecting there to be an opcode
  assert( cur_token != NULL );
  // get all the operands
  int i = 0;
  cur_token = strtok_r(NULL, ", ", &leftover);
  while( cur_token != NULL ) {
    strcpy(operands[i], cur_token);
    printf("%s ", cur_token);
    cur_token = strtok_r(NULL, ", ", &leftover);
    i++;
  }
  int numOperands = i;

  splitInstr out = {opcode, operands, numOperands};
  return out;
}

int main( void ) {
  char arr1[] = "add x0, x4, my_value";
  char arr2[] = ".int 0x84834";
  tokenize_line(arr1);
  tokenize_line(arr2);
}