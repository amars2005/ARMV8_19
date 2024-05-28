#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/*
call example: ./emulate <file_in>            - output to stdout
         alt: ./emulate <file_in> <file_out> - output to file

TODO:
1. Halt command
2. Add immediate
*/


#define MEM_SIZE (2 * (2 << 20)) // 2MB or 2*2^20 Bytes
#define GREG_NUM 31              // Number of general registers

// structure representing Processor State Register
typedef struct {
  atomic_bool N;
  atomic_bool Z;
  atomic_bool C;
  atomic_bool V;
} PSTATE;

// structure representing the state of the machine
typedef struct {
  uint8_t  memory[MEM_SIZE]; // Main Memory
  uint64_t R     [GREG_NUM]; // General Purpose Registers
  uint64_t PC              ; // Program Counter
  PSTATE   PSTATE          ; // Processor State
  const uint64_t ZR        ; // Zero Register
} state;

// sets the values of memory and registers to 0x0
static void setup(state* cstate) {
  for (int i; i < MEM_SIZE; i++) { cstate->memory[i] = 0; }
  for (int i; i < GREG_NUM; i++) { cstate->R[i]      = 0; }
  cstate->PC       = 0;
  cstate->PSTATE.Z = 1;
  cstate->PSTATE.C = 0;
  cstate->PSTATE.N = 0;
  cstate->PSTATE.V = 0;
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

void outputFile(state* cstate, char outputString[]) {
  for (int i = 0; i < GREG_NUM; i++) { //generates the line for the general registers
    uint64_t value = cstate->R[i];
    char line[LINE_STR_LENGTH];
    sprintf(line, "X%d = ", i);
    generateLine(value, line, outputString);
  }

  char pc[] = "PC = "; //generates the line for the program counter
  uint64_t value = cstate->PC;
  generateLine(value, pc, outputString);  
  char pstate[] = "PSTATE : "; //generates the line to be outputted for pstate

  if (cstate->PSTATE.Z==1) { strcat(pstate, "Z"); } else { strcat(pstate, "-"); }
  if (cstate->PSTATE.C==1) { strcat(pstate, "C"); } else { strcat(pstate, "-"); }
  if (cstate->PSTATE.N==1) { strcat(pstate, "N"); } else { strcat(pstate, "-"); }
  if (cstate->PSTATE.V==1) { strcat(pstate, "V\n"); } else { strcat(pstate, "-\n"); }
  strcat(outputString, pstate);

  for (int i = 0; i < MEM_SIZE; i++) { //checks non-zero memory and adds it to the output string
    if (cstate->memory[i] != 0) {
      char line[LINE_STR_LENGTH];
      sprintf(line, "%d = ", i);
      generateLine(cstate->memory[i], line, outputString); //adds any to the output
    }
  }
}
// stores contents of input binary file to memory of machine
static void loadfile(char fileName[], state* cstate) {
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
    cstate->memory[i] = getc(fp);
  }

  fclose(fp); // close file
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

void updateFlags(uint64_t result, PSTATE *pstate) {
  // Helper method to update the flags
  // N is set to sign bit of the result (not sure if this is correct)
  (*pstate).N = (result >> 63);
  if (result == 0) {
    (*pstate).Z = 1;
  }
  (*pstate).C = 0;
  (*pstate).V = 0;
}

void regAndFlags(uint64_t *rd, uint64_t *rn, uint64_t *op2, PSTATE *pstate) {
  // Bitwise AND on the values pointed to by rn and op2
  uint64_t result = *rn & *op2;
  *rd = result;
  updateFlags(result, pstate);
} 

void regClearFlags(uint64_t *rd, uint64_t *rn, uint64_t *op2, PSTATE *pstate) {
  // Bitwise BIC on rn and op2
  uint64_t result = *rn & ~(*op2);
  *rd = result;
  updateFlags(result, pstate);
} 

void regmAdd(uint64_t *rd, uint64_t *ra, uint64_t *rn, uint64_t *rm) {
  // Perform an mAdd on the values stored in ra, rn and rm
  *rd = *ra + ((*rn) * (*rm));
}

void regmSub(uint64_t *rd, uint64_t *ra, uint64_t *rn, uint64_t *rm) {
  // Perform an mSub on the values stored in ra, rn and rm
  *rd = *ra - ((*rn) * (*rm));
}

int main(int argc, char **argv) {
  // validate input arguments
  if (argc > 3 || argc < 2) { 
    fprintf(stderr, "Usage: emulate <file_in> [<file_out>]\n");
    exit(1);
  } 
  
  state cstate = { .ZR = 0 }; // initializes the machine
  setup(&cstate);

  loadfile(argv[1], &cstate);

  return EXIT_SUCCESS;
}