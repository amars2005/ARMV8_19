#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tokenizer.h"
#include "instruction-types.h"
#include "sdthandler.c"

// #include "symbol_table.h"

#define EQUAL_STRS(a,b) (strcmp((a),(b)) == 0)

// Don't use with an empty string please
splitLine tokenize_line(char *line_in, int instruction_address) {
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

  if (strcmp(opcode, "ldr") == 0 || strcmp(opcode, "str") == 0) {
      int i = 0;
      while (line_in[i] != ',') {
          i++;
      }
      i += 2;
      int addrIndex = i;
      char address[MAX_OPERAND_LENGTH];
      while (line_in[i] != '\0') {
          address[i - addrIndex] = line_in[i];
          i++;
      }
      address[i - addrIndex] = '\0';
      i--;
      while (line_in[i] == ' ') {
          address[i - addrIndex] = '\0';
          i--;
      }
      char operands2[MAX_OPERANDS][MAX_OPERAND_LENGTH];
      strcpy(operands2[0], operands[0]);
      strcpy(operands2[1], address);
      splitLine tokens = {opcode, operands2, 2, instruction_address};
      return tokens;
  }

  while( cur_token != NULL ) {
    strcpy(operands[i], cur_token);
    cur_token = strtok_r(NULL, ", ", &leftover);
    i++;
  }
  int num_operands = i;
  splitLine out = {opcode, operands, num_operands, instruction_address};
  return out;
}


instruction line_to_instruction(splitLine *data) {
  // Convert the operands to integers
  int *operands_as_ints[MAX_OPERANDS];
  // Check if any of the operands are a label
  for( int i = 0; i < data->num_operands; i++ ) {
    char *cur_operand = malloc(MAX_OPERAND_LENGTH * sizeof(char));
    strcpy(cur_operand, (data->operands)[i]);
    if( isLabel(cur_operand) ) {
      // Get the address of the label
      operands_as_ints[i] = find(SYMBOL_TABLE, cur_operand) - data->instruction_address;
    } else {
      // Convert it to an integer
      // Remove first character
      cur_operand++;
      *(operands_as_ints[i]) = atoi(cur_operand);
    }
    free(cur_operand);
  }

  // We need to deal with the case where op2 is a w register, x register or immediate
  if (EQUAL_STRS(data->opcode, "add")) {
      char *op2 = data->operands[2];

  } else if (EQUAL_STRS(data->opcode, "adds")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "sub")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "subs")) {
      //TODO();
  }else if (EQUAL_STRS(data->opcode, "cmp")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "cmn")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "neg")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "negs")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "and")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "ands")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "bic")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "bics")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "eor")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "eon")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "orr")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "orn")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "tst")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "mvn")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "mov")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "movn")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "movk")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "movz")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "madd")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "msub")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "mul")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "mneg")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "b")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "br")) {
      //TODO();
  // Deal with b.ne b.eq etc here (it isn't exactly b.cond)
  } else if (EQUAL_STRS(data->opcode, "b.cond")) {
      //TODO();
  } else if (EQUAL_STRS(data->opcode, "ldr")) {
      uint8_t sf;
      if (data->operands[0][0] == 'w') {
          sf = 0;
      } else {
          sf = 1;
      }
      char *rtTemp = &data->operands[0][1];
      uint8_t rt = (uint8_t) atoi(rtTemp);
      return SDTbuilder("ldr", rt, data->operands[1], sf);
  } else if (EQUAL_STRS(data->opcode, "str")) {
      uint8_t sf;
      if (data->operands[0][0] == 'w') {
          sf = 0;
      } else {
          sf = 1;
      }
      char *rtTemp = &data->operands[0][1];
      uint8_t rt = (uint8_t) atoi(rtTemp);
      return SDTbuilder("str", rt, data->operands[1], sf);
  } else if (EQUAL_STRS(data->opcode, ".int")) {
      //TODO();
  }
}

int main( void ) {
  char arr1[] = "add x0, x4, my_value";
  char arr2[] = ".int 0x84834";
  tokenize_line(arr1);
  tokenize_line(arr2);
}