#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <regex.h>

#include "tokenizer.h"
#include "instruction-types.h"
#include "bitwise-shift.h"

// #include "symbol_table.h"

#define ZR 31
#define EQUAL_STRS(a,b) (strcmp((a),(b)) == 0)

static uint64_t apply_shift(bool sf, uint64_t* rm, char* shift_str) {
    char shift_s[4];
    strncpy(shift_s, shift_str, 3);

    shiftType shift;
    if (EQUAL_STRS(shift_s, "lsl")) {
        shift = lsl;
    } else if (EQUAL_STRS(shift_s, "lsr")) {
        shift = lsr;
    } else if (EQUAL_STRS(shift_s, "asr")) {
        shift = asr;
    } else if (EQUAL_STRS(shift_s, "ror")) {
        shift = ror;
    }

    char *endptr;
    int amount = (int) strtol(shift_str + 5, &endptr, 10);

    return bitwiseShift(*rm, sf, shift, amount);
}

static bool isLabel(char* line) {
    char* label_regex_str = "[a-zA-Z_.][a-zA-Z0-9$_.]*";
    regex_t label_regex;
    int value = regcomp(&label_regex, label_regex_str, 0);

    if (value != 0) {
        fprintf(stderr, "Regex compilation failed\n");
        exit(1);
    }

    return (regexec(&label_regex, line, 0, NULL, 0) == REG_NOMATCH);
}


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

    while( cur_token != NULL ) {
        strcpy(operands[i], cur_token);
        cur_token = strtok_r(NULL, ", ", &leftover);
        i++;
    }
    int num_operands = i;
    splitLine out = {opcode, operands, num_operands, instruction_address};
    return out;
}

static void arith_dp_to_instruction(splitLine *data, uint64_t **operands_as_ints, bool sf, instruction *inst, arithmeticDPI_t opc) {
    if( data->num_operands == 4 ) {
        uint64_t shifted = apply_shift(sf, operands_as_ints[2], data->operands[3]);
        operands_as_ints[2] = &shifted;
    }
    char *op2 = data->operands[2];
    if( op2[0] == '#' ) {
        inst->instruction.arithmeticDpi.sf = sf;
        inst->instruction.arithmeticDpi.Rd = operands_as_ints[0];
        inst->instruction.arithmeticDpi.Rn = operands_as_ints[1];
        inst->instruction.arithmeticDpi.Op2 = *operands_as_ints[2];
        inst->instruction.arithmeticDpi.opc = opc;
        inst->itype = arithmeticDPIt;
    }
    else {
        inst->instruction.arithmeticDpr.sf = sf;
        inst->instruction.arithmeticDpr.Rd = operands_as_ints[0];
        inst->instruction.arithmeticDpr.Rn = operands_as_ints[1];
        inst->instruction.arithmeticDpr.Op2 = *operands_as_ints[2];
        inst->instruction.arithmeticDpr.opc = opc;
        inst->itype = arithmeticDPRt;
    }
}

static void logic_dpr_to_instruction(splitLine *data, uint64_t **operands_as_ints, bool sf, instruction *inst, logicDPR_t opc) {
    if( data->num_operands == 4 ) {
        uint64_t shifted = apply_shift(sf, operands_as_ints[2], data->operands[3]);
        operands_as_ints[2] = &shifted;
    }
    inst->instruction.logicDpr.sf = sf;
    inst->instruction.logicDpr.Rd = operands_as_ints[0];
    inst->instruction.logicDpr.Rn = operands_as_ints[1];
    inst->instruction.logicDpr.Op2 = *operands_as_ints[2];
    inst->instruction.logicDpr.opc = opc;
    inst->instruction.logicDpr.N = false;
    inst->itype = logicDPRt;
}

static void logic_dprn_to_instruction(splitLine *data, uint64_t **operands_as_ints, bool sf, instruction *inst, logicDPRN_t opc) {
    if( data->num_operands == 4 ) {
        uint64_t shifted = apply_shift(sf, operands_as_ints[2], data->operands[3]);
        operands_as_ints[2] = &shifted;
    }
    inst->instruction.logicDpr.sf = sf;
    inst->instruction.logicDpr.Rd = operands_as_ints[0];
    inst->instruction.logicDpr.Rn = operands_as_ints[1];
    inst->instruction.logicDpr.Op2 = *operands_as_ints[2];
    inst->instruction.logicDpr.opc = opc;
    inst->instruction.logicDpr.N = true;
    inst->itype = logicDPRt;
}

static void multiply_dpr_to_instruction(splitLine *data, uint64_t **operands_as_ints, bool sf, instruction *inst, bool x) {
    inst->instruction.multiplyDpr.sf = sf;
    inst->instruction.multiplyDpr.Rd = operands_as_ints[0];
    inst->instruction.multiplyDpr.Rn = operands_as_ints[1];
    inst->instruction.multiplyDpr.Rm = operands_as_ints[2];
    inst->instruction.multiplyDpr.Ra = operands_as_ints[3];
    inst->instruction.multiplyDpr.X = x;
    inst->itype = multiplyDPRt;
}

// Adds a new operand at zr_index that is the zero register, and shuffles the others up
static void add_zr_and_shuffle(uint64_t **operands_as_ints, int num_ops, int zr_index) {
    // Shuffle
    for (int i = num_ops; i >= zr_index; i--) {
        operands_as_ints[i] = operands_as_ints[i - 1];
    }
    // Add zr
    *(operands_as_ints[zr_index]) = ZR;
}

static void assemble_wmov(splitLine *data, uint64_t **operands_as_ints, bool sf, instruction *inst, arithmeticDPI_t opc) {
    if (data->num_operands == 3) {
        operands_as_ints[1] = apply_shift(sf, operands_as_ints[1], data->operands[2]);
    }
    if (data->operands[0] == '#') {
        inst->instruction.arithmeticDpi.sf = sf;
        inst->instruction.arithmeticDpi.Rd = operands_as_ints[0];
        inst->instruction.arithmeticDpi.Rn = operands_as_ints[1];
        inst->instruction.arithmeticDpi.Op2 = *operands_as_ints[2];
        inst->instruction.arithmeticDpi.opc = opc;
        inst->itype = arithmeticDPIt;
    }
}

instruction line_to_instruction(splitLine *data) {
    // Convert the operands to integers
    uint64_t *operands_as_ints[MAX_OPERANDS];
    // Check if any of the operands are a label
    for( int i = 0; i < data->num_operands; i++ ) {
        char *cur_operand = malloc(MAX_OPERAND_LENGTH * sizeof(char));
        strcpy(cur_operand, (data->operands)[i]);
        if( isLabel(cur_operand) ) {
            // Get the address of the label
            operands_as_ints[i] = find(SYMBOL_TABLE, cur_operand) - data->instruction_address;
        } else if( EQUAL_STRS((data->operands)[i], "xzr") || EQUAL_STRS((data->operands)[i], "wzr")) {
            // Zero register case
            *(operands_as_ints[i]) = ZR;
        } else {
            // Convert it to an integer
            // Remove first character
            cur_operand++;
            char *endptr;
            *(operands_as_ints[i]) = strtoull(cur_operand, &endptr, 10);
        }
        free(cur_operand);
    }

  // check first character to see whether its 32 or 64 bit
  bool sf;

  switch( data->operands[0][0] ) {
      case 'w':
          sf = false;
          break;
      case 'x':
          sf = true;
          break;
      default:
          fprintf(stderr, "register type of first operand expected to be w or x\n");
          exit(1);
  }

  // We need to deal with the case where op2 is a w register, x register or immediate
  instruction inst;
  if( EQUAL_STRS(data->opcode, "add") ) {
      arith_dp_to_instruction(data, operands_as_ints, sf, &inst, add);
  } else if (EQUAL_STRS(data->opcode, "adds")) {
      arith_dp_to_instruction(data, operands_as_ints, sf, &inst, adds);
  } else if (EQUAL_STRS(data->opcode, "sub")) {
      arith_dp_to_instruction(data, operands_as_ints, sf, &inst, sub);
  } else if (EQUAL_STRS(data->opcode, "subs")) {
      arith_dp_to_instruction(data, operands_as_ints, sf, &inst, subs);
  } else if (EQUAL_STRS(data->opcode, "cmp")) {
      // Shuffle operands down so I can put ZR in
      add_zr_and_shuffle(operands_as_ints, 3, 0);
      // Now we can call the equivalent dp case
      arith_dp_to_instruction(data, operands_as_ints, sf, &inst, subs);
  } else if (EQUAL_STRS(data->opcode, "cmn")) {
      // Shuffle operands down so I can put ZR in
      add_zr_and_shuffle(operands_as_ints, 3, 0);
      // Now we can call the equivalent dp case
      arith_dp_to_instruction(data, operands_as_ints, sf, &inst, adds);
  } else if (EQUAL_STRS(data->opcode, "neg")) {
      // Shuffle operands down so I can put ZR in
      add_zr_and_shuffle(operands_as_ints, 3, 1);
      // Now we can call the equivalent dp case
      arith_dp_to_instruction(data, operands_as_ints, sf, &inst, sub);
  } else if (EQUAL_STRS(data->opcode, "negs")) {
      // Shuffle operands down so I can put ZR in
      add_zr_and_shuffle(operands_as_ints, 3, 1);
      // Now we can call the equivalent dp case
      arith_dp_to_instruction(data, operands_as_ints, sf, &inst, subs);
  } else if (EQUAL_STRS(data->opcode, "and")) {
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, and);
  } else if (EQUAL_STRS(data->opcode, "ands")) {
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, ands);
  } else if (EQUAL_STRS(data->opcode, "bic")) {
      logic_dprn_to_instruction(data, operands_as_ints, sf, &inst, bic);
  } else if (EQUAL_STRS(data->opcode, "bics")) {
      logic_dprn_to_instruction(data, operands_as_ints, sf, &inst, bics);
  } else if (EQUAL_STRS(data->opcode, "eor")) {
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, eor);
  } else if (EQUAL_STRS(data->opcode, "eon")) {
      logic_dprn_to_instruction(data, operands_as_ints, sf, &inst, eon);
  } else if (EQUAL_STRS(data->opcode, "orr")) {
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, orr);
  } else if (EQUAL_STRS(data->opcode, "orn")) {
      logic_dprn_to_instruction(data, operands_as_ints, sf, &inst, orn);
  } else if (EQUAL_STRS(data->opcode, "tst")) {
      // Shuffle operands down so I can put ZR in
      add_zr_and_shuffle(operands_as_ints, 3, 0);
      // Now we can call the equivalent dp case
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, ands);
  } else if (EQUAL_STRS(data->opcode, "mvn")) {
      TODO();
  } else if (EQUAL_STRS(data->opcode, "movn") || EQUAL_STRS(data->opcode, "movk") || EQUAL_STRS(data->opcode, "movz")) {
      char* opc = data->opcode;
      int rd = operands_as_ints[0];
      int imm16 = operands_as_ints[1];
      int hw;
      if (data->num_operands > 2) {
        hw = operands_as_ints[2];
      } else {
        hw = 0
      }
      assembleWideMoveDPI(opc, rd, imm16, hw, sf);
  } else if (EQUAL_STRS(data->opcode, "mov")) {
      TODO();
  } else if (EQUAL_STRS(data->opcode, "madd")) {
      multiply_dpr_to_instruction(data, operands_as_ints, sf, &inst, false);
  } else if (EQUAL_STRS(data->opcode, "msub")) {
      multiply_dpr_to_instruction(data, operands_as_ints, sf, &inst, true);
  } else if (EQUAL_STRS(data->opcode, "mul")) {
      *operands_as_ints[3] = ZR;
      multiply_dpr_to_instruction(data, operands_as_ints, sf, &inst, false);
  } else if (EQUAL_STRS(data->opcode, "mneg")) {
      *operands_as_ints[3] = ZR;
      multiply_dpr_to_instruction(data, operands_as_ints, sf, &inst, true);
  } else if (EQUAL_STRS(data->opcode, "b")) {
      int simm26 = operands_as_ints[0];
      unCondBranch(simm26);
  } else if (EQUAL_STRS(data->opcode, "br")) {
      int xn = operands_as_ints[0];
      registerBranch(xn);
  } else if (EQUAL_STRS(data->opcode, "b.eq") || EQUAL_STRS(data->opcode, "b.ne") || EQUAL_STRS(data->opcode, "b.ge") || EQUAL_STRS(data->opcode, "b.lt") || EQUAL_STRS(data->opcode, "b.gt") || EQUAL_STRS(data->opcode, "b.le") || EQUAL_STRS(data->opcode, "b.al")) {
      int simm19 = operands_as_ints[0];
      char* cond = data->operands[1];
      condBranch(simm19, cond); 
  } else if (EQUAL_STRS(data->opcode, "ldr")) {
      TODO();
  } else if (EQUAL_STRS(data->opcode, "str")) {
      TODO();
  } else if (EQUAL_STRS(data->opcode, ".int")) {
      TODO();
  }
  return inst;
}

int main( void ) {
  char arr1[] = "opc x0, x4, my_value";
  char arr2[] = ".int 0x84834";
  // tokenize_line(arr1);
  // tokenize_line(arr2);
}