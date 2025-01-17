#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <regex.h>
#include <ctype.h>
#include "tokenizer.h"
//include "instruction-types.h"
#include "bitwise-shift.h"
//#include "sdthandler.h"
//#include "symbol_table.h"

bool isLabel(char* line) {
    char* label_regex_str = "^[a-zA-Z_.][a-zA-Z0-9$_.]*$";
    regex_t label_regex;
    int value = regcomp(&label_regex, label_regex_str, REG_EXTENDED);

    if (value != 0) {
        fprintf(stderr, "Regex compilation failed\n");
        exit(1);
    }

    bool ret = (regexec(&label_regex, line, 0, NULL, 0) != REG_NOMATCH);
    regfree(&label_regex);
    return ret;
}

bool isLabelColon(char *line) {
    char* label_regex_str = "^[a-zA-Z_.][a-zA-Z0-9$_.]*:[ ]*$";
    regex_t label_regex;
    int value = regcomp(&label_regex, label_regex_str, REG_EXTENDED);

    if (value != 0) {
        fprintf(stderr, "Regex compilation failed\n");
        exit(1);
    }

    bool ret = (regexec(&label_regex, line, 0, NULL, 0) != REG_NOMATCH);
    regfree(&label_regex);
    return ret;
}

char *addSpaces(char *line) {
    int new_length = 0;
    for (int i = 0; line[i] != '\0'; i++) {
        new_length++;
        if (line[i] == ',') {
            new_length++;
        }
    }

    // Allocate memory for the new string
    char *result = (char *)malloc(MAX_LINE_LENGTH);
    assert(result != NULL);

    // Build the new string
    int j = 0;
    for (int i = 0; line[i] != '\0'; i++) {
        result[j++] = line[i];
        if (line[i] == ',') {
            result[j++] = ' ';
        }
    }
    result[j] = '\0';

    return result;
}

static shift workoutShift(char* operand, int num_operands) {
    shiftType shiftt;
    int       amount;
    if (num_operands == 4) {
        char shift_s[4];
        strncpy(shift_s, operand, 3);

        if (EQUAL_STRS(shift_s, "lsl")) {
            shiftt = lsl;
        } else if (EQUAL_STRS(shift_s, "lsr")) {
            shiftt = lsr;
        } else if (EQUAL_STRS(shift_s, "asr")) {
            shiftt = asr;
        } else if (EQUAL_STRS(shift_s, "ror")) {
            shiftt = ror;
        } else {
            fprintf(stderr, "Invalid shift\n");
            exit(1);
        }
        char *endptr;
        operand += 5;
        if (operand[1] == 'x') { amount = strtoull(operand, &endptr, 0); }
        else { amount = (int) strtol(operand, &endptr, 10); }
    } else {
        shiftt = 0;
        amount = 0;
    }
    shift s = { .stype = shiftt, .amount = amount };
    return s;
}

// Don't use with an empty string please
splitLine tokenize_line(char *line_in, int instruction_address) {
  char *line = (char *)malloc(MAX_LINE_LENGTH);
  strcpy(line, line_in);
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
  cur_token = strtok_r(NULL, ",", &leftover);

  if (!strcmp(opcode, "ldr") || !strcmp(opcode, "str")) {
      while (line[i] != ',') {
          i++;
      }
      i++;
      if (isspace(line[i]) == false) {
        char *newline = addSpaces(line);
        strcpy(line, newline);
        free(newline);
      }
      i++;

      int addrIndex = i;
      char address[MAX_OPERAND_LENGTH];
      while (line[i] != '\0') {
          address[i - addrIndex] = line[i];
          i++;
      }
      address[i - addrIndex] = '\0';
      i--;
      while (line[i] == ' ') {
          address[i - addrIndex] = '\0';
          i--;
      }
      char operands2[MAX_OPERANDS][MAX_OPERAND_LENGTH];
      strcpy(operands2[0], cur_token);
      strcpy(operands2[1], address);
      splitLine tokens = {opcode, operands2, 2, instruction_address};
      free(line);
      return tokens;
  }

  while( cur_token != NULL ) {
    if (cur_token[0] == ' ') { cur_token++; }
    strcpy(operands[i], cur_token);
    cur_token = strtok_r(NULL, ",", &leftover);
    i++;
  }
  int num_operands = i;
  splitLine out = {opcode, operands, num_operands, instruction_address};
  free(line);
  return out;
}

static void arith_dp_to_instruction(splitLine *data, uint64_t *operands_as_ints, bool sf, instruction *inst, arithmeticDPI_t opc) {
    if( data->operands[2][0] == '#' ) {
        bool sh = !strcmp(data->operands[3], "lsl #12");
        inst->instruction.arithmeticDpi.sf = sf;
        inst->instruction.arithmeticDpi.Rd = &operands_as_ints[0];
        inst->instruction.arithmeticDpi.Rn = &operands_as_ints[1];
        inst->instruction.arithmeticDpi.Op2 = operands_as_ints[2];
        inst->instruction.arithmeticDpi.sh  = sh;
        inst->instruction.arithmeticDpi.opc = opc;
        inst->itype = arithmeticDPIt;
    }
    else {
        shift shift = workoutShift(data->operands[3], data->num_operands);
        inst->instruction.arithmeticDpr.sf = sf;
        inst->instruction.arithmeticDpr.Rd = &operands_as_ints[0];
        inst->instruction.arithmeticDpr.Rn = &operands_as_ints[1];
        inst->instruction.arithmeticDpr.Rm = &operands_as_ints[2];
        inst->instruction.arithmeticDpr.Shift = shift.stype;
        inst->instruction.arithmeticDpr.Operand = shift.amount;
        inst->instruction.arithmeticDpr.opc = opc;
        inst->itype = arithmeticDPRt;
    }
}

static void logic_dpr_to_instruction(splitLine *data, uint64_t *operands_as_ints, bool sf, instruction *inst, uint64_t opc, bool n) {
    shift shift = workoutShift(data->operands[3], data->num_operands);

    inst->instruction.logicDpr.sf = sf;
    inst->instruction.logicDpr.Rd = &operands_as_ints[0];
    inst->instruction.logicDpr.Rn = &operands_as_ints[1];
    inst->instruction.logicDpr.Rm = &operands_as_ints[2];
    inst->instruction.logicDpr.Shift = shift.stype;
    inst->instruction.logicDpr.Operand = shift.amount;
    inst->instruction.logicDpr.opc = opc;
    inst->instruction.logicDpr.N = n;
    inst->itype = logicDPRt;
}

static void multiply_dpr_to_instruction(splitLine *data, uint64_t *operands_as_ints, bool sf, instruction *inst, bool x) {
    inst->instruction.multiplyDpr.sf = sf;
    inst->instruction.multiplyDpr.Rd = &operands_as_ints[0];
    inst->instruction.multiplyDpr.Rn = &operands_as_ints[1];
    inst->instruction.multiplyDpr.Rm = &operands_as_ints[2];
    inst->instruction.multiplyDpr.Ra = &operands_as_ints[3];
    inst->instruction.multiplyDpr.X = x;
    inst->itype = multiplyDPRt;
}

// Adds a new operand at zr_index that is the zero register, and shuffles the others up
static void add_zr_and_shuffle(uint64_t *operands_as_ints, splitLine *data, int zr_index, bool sf) {
    // Shuffle
    for (int i = data->num_operands; i >= zr_index; i--) {
        operands_as_ints[i] = operands_as_ints[i - 1];
        strcpy(data->operands[i], data->operands[i - 1]);
    }
    data->num_operands++;
    // Add zr
    operands_as_ints[zr_index] = ZR;
    if (sf) { strcpy(data->operands[zr_index], "xzr"); }
    else { strcpy(data->operands[zr_index], "wzr"); }
}

static void assemble_wmov(splitLine *data, uint64_t *operands_as_ints, bool sf, instruction *inst, int opc) {
    inst->instruction.wideMoveDpi.sf = sf;
    inst->instruction.wideMoveDpi.Rd = &operands_as_ints[0];
    inst->instruction.wideMoveDpi.Op = operands_as_ints[1];
    inst->instruction.wideMoveDpi.opc = opc;
    inst->itype = wideMoveDPIt;
    if (data->num_operands == 3) {
        uint64_t hw = atoi(data->operands[2] + 5);
        inst->instruction.wideMoveDpi.hw = hw;
    }
    else {
        inst->instruction.wideMoveDpi.hw = 0;
    }
}

instruction line_to_instruction(splitLine *data, symbolt symbol_table) {
    // Convert the operands to integers
    uint64_t operands_as_ints[MAX_OPERANDS];
    char operands[MAX_OPERANDS][MAX_OPERAND_LENGTH];
    int op1_length = strlen(data->operands[1]);
    for (int i = 0; i < data->num_operands; i++) {
        strcpy(operands[i], data->operands[i]);
    }
    // Check if any of the operands are a label
    if (strcmp(data->opcode,"ldr") && strcmp(data->opcode,"str")) {
        for( int i = 0; i < data->num_operands; i++ ) {
            char *cur_operand = malloc(MAX_OPERAND_LENGTH * sizeof(char));
            strcpy(cur_operand, (operands)[i]);

            if( EQUAL_STRS((operands)[i], "xzr") || EQUAL_STRS((operands)[i], "wzr")) {
                // Zero register case
                operands_as_ints[i] = ZR;
            } else if ((cur_operand[0] == 'x' || cur_operand[0] == 'w' || cur_operand[0] == '#') & (find(symbol_table, cur_operand) == -1)) {
                // Convert it to an integer
                // Remove first character
                char *endptr;
                if (cur_operand[2] == 'x') { operands_as_ints[i] = strtoull(cur_operand + 1, &endptr, 0); }
                else { operands_as_ints[i] = strtoull(cur_operand + 1, &endptr, 10); }
            } else if( isLabel(cur_operand)) {
                // Get the address of the label
                operands_as_ints[i] = find(symbol_table, cur_operand);
            } else if (!strcmp(data->opcode, ".int")) {
                char *endptr;
                if (cur_operand[1] == 'x') { operands_as_ints[i] = strtoull(cur_operand, &endptr, 0); }
                else { operands_as_ints[i] = strtoull(cur_operand, &endptr, 10); }
            }
            free(cur_operand);
        }
    }    

  // check first character to see whether its 32 or 64 bit
  bool sf;

  switch( operands[0][0] ) {
      case 'w':
          sf = false;
          break;
      case 'x':
          sf = true;
          break;
      default:
          if (isLabel(operands[0]) || isLabelColon(data->opcode)) {
            sf = 1;
            break;
          }
          if (strcmp(data->opcode, ".int")) {
            fprintf(stderr, "register type of first operand expected to be w or x\n");
            exit(1);
          } else {
            sf = false;
            break;
          }
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
      add_zr_and_shuffle(operands_as_ints, data, 0, sf);
      // Now we can call the equivalent dp case
      arith_dp_to_instruction(data, operands_as_ints, sf, &inst, subs);
  } else if (EQUAL_STRS(data->opcode, "cmn")) {
      // Shuffle operands down so I can put ZR in
      add_zr_and_shuffle(operands_as_ints, data, 0, sf);
      // Now we can call the equivalent dp case
      arith_dp_to_instruction(data, operands_as_ints, sf, &inst, adds);
  } else if (EQUAL_STRS(data->opcode, "neg")) {
      // Shuffle operands down so I can put ZR in
      add_zr_and_shuffle(operands_as_ints, data, 1, sf);
      // Now we can call the equivalent dp case
      arith_dp_to_instruction(data, operands_as_ints, sf, &inst, sub);
  } else if (EQUAL_STRS(data->opcode, "negs")) {
      // Shuffle operands down so I can put ZR in
      add_zr_and_shuffle(operands_as_ints, data, 1, sf);
      // Now we can call the equivalent dp case
      arith_dp_to_instruction(data, operands_as_ints, sf, &inst, subs);
  } else if (EQUAL_STRS(data->opcode, "and")) {
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, and, false);
  } else if (EQUAL_STRS(data->opcode, "ands")) {
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, ands, false);
  } else if (EQUAL_STRS(data->opcode, "bic")) {
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, bic, true);
  } else if (EQUAL_STRS(data->opcode, "bics")) {
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, bics, true);
  } else if (EQUAL_STRS(data->opcode, "eor")) {
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, eor, false);
  } else if (EQUAL_STRS(data->opcode, "eon")) {
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, eon, true);
  } else if (EQUAL_STRS(data->opcode, "orr")) {
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, orr, false);
  } else if (EQUAL_STRS(data->opcode, "orn")) {
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, orn, true);
  } else if (EQUAL_STRS(data->opcode, "tst")) {
      // Shuffle operands down so I can put ZR in
      add_zr_and_shuffle(operands_as_ints, data, 0, sf);
      // Now we can call the equivalent dp case
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, ands, false);
  } else if (EQUAL_STRS(data->opcode, "mvn")) {
      add_zr_and_shuffle(operands_as_ints, data, 1, sf);
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, orn, true);
  } else if (EQUAL_STRS(data->opcode, "movn")) {
    //assemble_wmov(splitLine *data, uint64_t **operands_as_ints, bool sf, instruction *inst, arithmeticDPI_t opc)
      assemble_wmov(data, operands_as_ints, sf, &inst, movn);
  } else if (EQUAL_STRS(data->opcode, "movk")) {
      assemble_wmov(data, operands_as_ints, sf, &inst, movk);
  } else if (EQUAL_STRS(data->opcode, "movz")) {
      assemble_wmov(data, operands_as_ints, sf, &inst, movz);
  } else if (EQUAL_STRS(data->opcode, "mov")) {
      add_zr_and_shuffle(operands_as_ints, data, 1, sf);
      logic_dpr_to_instruction(data, operands_as_ints, sf, &inst, orr, false);
  } else if (EQUAL_STRS(data->opcode, "madd")) {
      multiply_dpr_to_instruction(data, operands_as_ints, sf, &inst, false);
  } else if (EQUAL_STRS(data->opcode, "msub")) {
      multiply_dpr_to_instruction(data, operands_as_ints, sf, &inst, true);
  } else if (EQUAL_STRS(data->opcode, "mul")) {
      operands_as_ints[3] = ZR;
      multiply_dpr_to_instruction(data, operands_as_ints, sf, &inst, false);
  } else if (EQUAL_STRS(data->opcode, "mneg")) {
      operands_as_ints[3] = ZR;
      multiply_dpr_to_instruction(data, operands_as_ints, sf, &inst, true);
  } else if (EQUAL_STRS(data->opcode, "b")) {
      inst.itype = brancht;
      inst.instruction.branch.offset = operands_as_ints[0];
      inst.instruction.branch.offset -= data->instruction_address;
  } else if (EQUAL_STRS(data->opcode, "br")) {
      inst.itype = bregt;
      inst.instruction.breg.Xn = &operands_as_ints[0];
  } else if (EQUAL_STRS(data->opcode, "b.eq")) {
      inst.itype = bcondt;
      inst.instruction.bcond.offset = operands_as_ints[0];
      inst.instruction.bcond.offset -= data->instruction_address;
      inst.instruction.bcond.cond   = EQ;
  } else if (EQUAL_STRS(data->opcode, "b.ne")) {
      inst.itype = bcondt;
      inst.instruction.bcond.offset = operands_as_ints[0];
      inst.instruction.bcond.offset -= data->instruction_address;
      inst.instruction.bcond.cond   = NE;
  } else if (EQUAL_STRS(data->opcode, "b.ge")) {
      inst.itype = bcondt;
      inst.instruction.bcond.offset = operands_as_ints[0];
      inst.instruction.bcond.offset -= data->instruction_address;
      inst.instruction.bcond.cond   = GE;
  } else if (EQUAL_STRS(data->opcode, "b.lt")) {
      inst.itype = bcondt;
      inst.instruction.bcond.offset = operands_as_ints[0];
      inst.instruction.bcond.offset -= data->instruction_address;
      inst.instruction.bcond.cond   = LT;
  } else if (EQUAL_STRS(data->opcode, "b.gt")) {
      inst.itype = bcondt;
      inst.instruction.bcond.offset = operands_as_ints[0];
      inst.instruction.bcond.offset -= data->instruction_address;
      inst.instruction.bcond.cond   = GT;
  } else if (EQUAL_STRS(data->opcode, "b.le")) {
      inst.itype = bcondt;
      inst.instruction.bcond.offset = operands_as_ints[0];
      inst.instruction.bcond.offset -= data->instruction_address;
      inst.instruction.bcond.cond   = LE;
  } else if (EQUAL_STRS(data->opcode, "b.al")) {
      inst.itype = bcondt;
      inst.instruction.bcond.offset = operands_as_ints[0];
      inst.instruction.bcond.offset -= data->instruction_address;
      inst.instruction.bcond.cond   = AL;
  } else if (EQUAL_STRS(data->opcode, "ldr")) {
      data->operands[1][op1_length] = '\0';
      char *rtTemp = &data->operands[0][1];
      uint8_t rt = (uint8_t) atoi(rtTemp);
      inst = SDTbuilder("ldr", rt, data->operands[1], sf, symbol_table, data->instruction_address);
  } else if (EQUAL_STRS(data->opcode, "str")) {
      data->operands[1][op1_length] = '\0';
      char *rtTemp = &data->operands[0][1];
      uint8_t rt = (uint8_t) atoi(rtTemp);
      inst = SDTbuilder("str", rt, data->operands[1], sf, symbol_table, data->instruction_address);
  } else if (EQUAL_STRS(data->opcode, ".int")) {
      inst.itype = directive;
      inst.instruction.directive = LITTLE(operands_as_ints[0]);
  } else if (isLabelColon(data->opcode)) {
      inst.itype = label;
  } else {
      fprintf(stderr, "unexpected opcode\n");
      exit(1);
  }
  return inst;
}