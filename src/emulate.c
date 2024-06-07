#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

/*
call example: ./emulate <file_in>            - output to stdout
         alt: ./emulate <file_in> <file_out> - output to file

TODO:
1. Halt command
2. Add immediate
*/


#define MEM_SIZE (2 * (2 << 20)) // 2MB or 2*2^20 Bytes
#define GREG_NUM 31              // Number of general registers

#define EQ 0 // 0000
#define NE 1 // 0001
#define GE 10 // 1010
#define LT 11 // 1011
#define GT 12 // 1100
#define LE 13 // 1101
#define AL 14 // 1110

// structure representing Processor State Register
typedef struct {
  atomic_bool N;
  atomic_bool Z;
  atomic_bool C;
  atomic_bool V;
} PSTATE;

// structure representing the state of the machine
struct {
  uint8_t  memory[MEM_SIZE]; // Main Memory
  uint64_t R     [GREG_NUM]; // General Purpose Registers
  uint64_t PC              ; // Program Counter
  PSTATE   PSTATE          ; // Processor State
  const uint64_t ZR        ; // Zero Register
} state = { .ZR = 0 };

// sets the values of memory and registers to 0x0
static void setup(void) {
  for (int i; i < MEM_SIZE; i++) { state.memory[i] = 0; }
  for (int i; i < GREG_NUM; i++) { state.R[i]      = 0; }
  state.PC       = 0;
  state.PSTATE.Z = 1;
  state.PSTATE.C = 0;
  state.PSTATE.N = 0;
  state.PSTATE.V = 0;
}

#define VALUE_STR_LENGTH 16
#define LINE_STR_LENGTH 50

char* valueToStr(char* valueAsStr, uint64_t value) {
  sprintf(valueAsStr, "%lx", value); //creates the value as a string

  if (strlen(valueAsStr) < VALUE_STR_LENGTH) {
    int zeroes = VALUE_STR_LENGTH - strlen(valueAsStr);
    char *zeroString = malloc(VALUE_STR_LENGTH);

    for (int i = 0; i < zeroes; i++) { //adds enough zeroes to make it 16 digits
      strcat(zeroString, "0");
    }

    strcat(zeroString, valueAsStr);
    valueAsStr = zeroString;
  }
  return valueAsStr;
}

void generateLine(uint64_t value, char line[], char outputString[]) {
  char valueAsStr[VALUE_STR_LENGTH]; //creates a line with the register name
  char x[16];
  sprintf(x, "%s", valueToStr(valueAsStr, value));
  strcat(line, x); //adds the value to the line
  strcat(line, "\n");
  strcat(outputString, line); //adds the line to the string
}

void outputFile(char outputString[]) {
  for (int i = 0; i < GREG_NUM; i++) { //generates the line for the general registers
    uint64_t value = state.R[i];
    char line[LINE_STR_LENGTH];
    sprintf(line, "X%d = ", i);
    generateLine(value, line, outputString);
  }

  char pc[] = "PC = "; //generates the line for the program counter
  uint64_t value = state.PC;
  generateLine(value, pc, outputString);  
  char pstate[] = "PSTATE : "; //generates the line to be outputted for pstate

  if (state.PSTATE.Z==1) { strcat(pstate, "Z"); } else { strcat(pstate, "-"); }
  if (state.PSTATE.C==1) { strcat(pstate, "C"); } else { strcat(pstate, "-"); }
  if (state.PSTATE.N==1) { strcat(pstate, "N"); } else { strcat(pstate, "-"); }
  if (state.PSTATE.V==1) { strcat(pstate, "V\n"); } else { strcat(pstate, "-\n"); }
  strcat(outputString, pstate);

  for (int i = 0; i < MEM_SIZE; i++) { //checks non-zero memory and adds it to the output string
    if (state.memory[i] != 0) {
      char line[LINE_STR_LENGTH];
      sprintf(line, "%d = ", i);
      generateLine(state.memory[i], line, outputString); //adds any to the output
    }
  }
}

// stores contents of input binary file to memory of machine
static void loadfile(char fileName[]) {
  FILE *fp = fopen(fileName, "rb"); // open file

  // check if file exists
  if (fp == NULL) {
    fprintf(stderr, "emulate: can't open %s\n", fileName);
    exit(1);
  }

  // caclulate length of file
  fseek(fp, 1, SEEK_END);
  long fileSize = ftell(fp);
  rewind(fp);

  // check that program fits in memory
  if (fileSize >= MEM_SIZE) {
    fprintf(stderr, "emulate: %s is greater than 2MB\n", fileName);
    exit(1);
  }

  //read file and store in memory
  for (int i = 0; i < (fileSize-1); i++) {
    state.memory[i] = getc(fp);
  }

  fclose(fp); // close file
}

// Functions for data processing instructions using immediate addressing

void immAdd(uint64_t *rd, uint64_t *rn, uint64_t *imm12, bool w) {
  // Bitwise ADD on the values pointed to by rn and imm12
  *rd = *rn + *imm12;
} 

void immAddFlags(uint64_t *rd, uint64_t *rn, uint64_t *imm12, PSTATE *pstate, bool z) {
  // Bitwise ADD on the values pointed to by rn and imm12
  int result = *rn + *imm12;
  *rd = result;

  int bits;
  if (!z) { int bits = 32; }
  else { int bits = 64; }

  (*pstate).N = (result >> (bits - 1));
  if (result == 0) { (*pstate).Z = 1; }
  if (result > (2 << bits)) { (*pstate).C = 1; }
  if (result > (2 << (bits - 1))) { (*pstate).V = 1; }
}

void immSub(uint64_t *rd, uint64_t *rn, uint64_t *imm12) {
  // Bitwise SUB on the values pointed to by rn and imm12
  *rd = *rn - *imm12;
} 

void immSubFlags(uint64_t *rd, uint64_t *rn, uint64_t *imm12, PSTATE *pstate, bool z) {
  // Bitwise SUB on the values pointed to by rn and imm12
  int result = *rn - *imm12;
  *rd = result;
  
  int bits;
  if (!z) { int bits = 32; }
  else { int bits = 64; }

  (*pstate).N = (result >> (bits - 1));
  if (result == 0) { (*pstate).Z = 1; }
  if ((*pstate).N) { (*pstate).C = 1; }
  if (result > (2 << (bits - 1))) { (*pstate).V = 1; }
}

void wMovN(uint64_t *rd, uint64_t *hw, uint64_t *imm16, bool z) {
  //Sets the value in rd to the bitwise negation of imm16
  int bits;
  if (!z) { int bits = 32; }
  else { int bits = 64; }

  *rd = (2 << bits) + imm16 - (2 << (int) hw);
}

void wMovZ(uint64_t *rd, uint64_t *imm16) {
  //Sets the value in rd to imm16
  *rd = *imm16;
}

void wMovK(uint64_t *rd, uint64_t *hw, uint64_t *imm16, bool z) {
  //Inserts the value of imm16 into rd, keeping all the other bits the same.
  int bits;
  if (!z) { int bits = 32; }
  else { int bits = 64; }

  *rd = *rd - bits(*rd, *hw, *hw + 15) + (*imm16 * 2 << *hw);
}

// Functions for data processing instructions with registers

void regAnd(uint64_t *rd, uint64_t *rn, uint64_t *op2) {
  // Bitwise AND on the values pointed to by rn and op2
  *rd = *rn & *op2;
} 

void regClear(uint64_t *rd, uint64_t *rn, uint64_t *op2) {
  // BIC is the same as AND with the complement of the second operand
  *rd = *rn & ~(*op2);
}

void regOr(uint64_t *rd, uint64_t *rn, uint64_t *op2) {
  // Bitwise OR on the values pointed to by rn and op2
  *rd = *rn | *op2;
}

void regOrn(uint64_t *rd, uint64_t *rn, uint64_t *op2) {
  // Bitwise OR with the complement of the second operand
  *rd = *rn | ~(*op2);
}

void regXor(uint64_t *rd, uint64_t *rn, uint64_t *op2) {
  // Bitwise XOR on the values pointed to by rn and op2
  *rd = *rn ^ *op2;
}

void regXorn(uint64_t *rd, uint64_t *rn, uint64_t *op2) {
  // Bitwise XOR with the complement of the second operand
  *rd = *rn ^ ~(*op2);
}

void updateFlags(uint64_t result) {
  // Helper method to update the flags
  // N is set to sign bit of the result (not sure if this is correct)
  state.PSTATE.N = (result >> 63);
  if (result == 0) {
    state.PSTATE.Z = 1;
  }
  state.PSTATE.C = 0;
  state.PSTATE.V = 0;
}

void regAndFlags(uint64_t *rd, uint64_t *rn, uint64_t *op2) {
  // Bitwise AND on the values pointed to by rn and op2
  uint64_t result = *rn & *op2;
  *rd = result;
  updateFlags(result);
} 

void regClearFlags(uint64_t *rd, uint64_t *rn, uint64_t *op2) {
  // Bitwise BIC on rn and op2
  uint64_t result = *rn & ~(*op2);
  *rd = result;
  updateFlags(result);
} 

void regmAdd(uint64_t *rd, uint64_t *ra, uint64_t *rn, uint64_t *rm) {
  // Perform an mAdd on the values stored in ra, rn and rm
  *rd = *ra + ((*rn) * (*rm));
}

void regmSub(uint64_t *rd, uint64_t *ra, uint64_t *rn, uint64_t *rm) {
  // Perform an mSub on the values stored in ra, rn and rm
  *rd = *ra - ((*rn) * (*rm));
}

// Functions for branch instructions (1.8)

void unCondBranch(uint64_t offset) {
  // Apply the offset to the PC
  state.PC += offset; 
}

void registerBranch(uint64_t *xn) {
  // Branch directly to the address stored in xn
  state.PC = (*xn);
}

void condBranch(uint64_t offset, uint64_t cond) {
  // Apply the offset to the PC iff cond is satisfied by PSTATE
  bool condEval;
  switch (cond) {
    case EQ:
      condEval = (state.PSTATE.Z == 1);
      break;
    case NE:
      condEval = (state.PSTATE.Z == 0);
      break;
    case GE:
      condEval = (state.PSTATE.N == state.PSTATE.V);
      break;
    case LT:
      condEval = (state.PSTATE.N != state.PSTATE.V);
      break;
    case GT:
      condEval = (state.PSTATE.N == state.PSTATE.V) && (state.PSTATE.Z == 0);
      break;
    case LE:
      condEval = !((state.PSTATE.N == state.PSTATE.V) && (state.PSTATE.Z == 0));
      break;
    case AL:
      condEval = true;
      break;
    default:
      // undefined behaviour
      fprintf(stderr, "Condition with code %ld is not defined", cond);
  }
  if (condEval) {
    unCondBranch(offset);
  }
}

//Function for halt 1.9

int halt() {
  exit(0);
}

int main(int argc, char **argv) {
  // validate input arguments
  if (argc > 3 || argc < 2) { 
    fprintf(stderr, "Usage: emulate <file_in> [<file_out>]\n");
    exit(1);
  } 
  
  setup();
  loadfile(argv[1]);

  return EXIT_SUCCESS;
}