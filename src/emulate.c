#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

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


// an2823 working with functions (and all the code written up above)

void lsl32(uint64_t *rn, int shift_amount) {
  uint32_t lowerBits = (uint32_t) (*rn & 0xFFFFFFFF);
  *rn = (uint64_t) (lowerBits << shift_amount);
}

void lsr32(uint64_t *rn, int shift_amount) {
  uint32_t lowerBits = (uint32_t) (*rn & 0xFFFFFFFF);
  *rn = (uint64_t) (lowerBits >> shift_amount);
}

void asr32(uint64_t *rn, int shift_amount) {
  uint32_t lowerBits = (uint32_t) (*rn & 0xFFFFFFFF);
  uint32_t sign = lowerBits & 0x80000000;
  for (int i = 0; i < shift_amount; i++) {
    lowerBits >>= 1;
    lowerBits |= sign;
  }
  *rn = (uint64_t) lowerBits;
}

void ror32(uint64_t *rn, int shift_amount) {
  uint32_t lowerBits = (uint32_t) (*rn & 0xFFFFFFFF);
  for (int i = 0; i < shift_amount; i++) {
    uint32_t lsb = lowerBits & 0x00000001;
    lowerBits >>= 1;
    if (lsb != 0) {
      lowerBits |= 0x80000000;
    } else {
      lowerBits &= 0x7FFFFFFF;
    }
  }
  *rn = lowerBits;
}

void lsl64(uint64_t *rn, int shift_amount) {
  *rn <<= shift_amount;
}

void lsr64(uint64_t *rn, int shift_amount) {
  *rn >>= shift_amount;
}

void asr64(uint64_t *rn, int shift_amount) {
  uint64_t sign = *rn & 0x8000000000000000;
  for (int i = 0; i < shift_amount; i++) {
    *rn >>= 1;
    *rn = *rn | sign;
  }
}

void ror64(uint64_t *rn, int shift_amount) {
  for (int i = 0; i < shift_amount; i++) {
    uint64_t lsb = *rn & 0x0000000000000001;
    *rn >>= 1;
    if (lsb != 0) {
      *rn |= 0x8000000000000000;
    } else {
      *rn &= 0x7FFFFFFFFFFFFFFF;
    }
  } 
}


// First input is the register number
// Second input is the register mode (32 or 64 bit) represented by 0 and 1 respectively
// Third input is the instruction type (lsl, lsr, asr, ror)
// 0, 1, 2, 3 for lsl, lsr, asr, ror respectively
void bitwiseShift(uint64_t *rn, int mode, int instruction, int shift_amount) {
  assert(instruction >= 0 && instruction <= 3);

  assert(mode == 0 || mode == 1);

  if (mode == 0) {
    switch (instruction) {
      case 0: lsl32(rn, shift_amount); break;
      case 1: lsr32(rn, shift_amount); break;
      case 2: asr32(rn, shift_amount); break;
      case 3: ror32(rn, shift_amount); break;
    }
  } else {
    switch (instruction) {
      case 0: lsl64(rn, shift_amount); break;
      case 1: lsr64(rn, shift_amount); break;
      case 2: asr64(rn, shift_amount); break;
      case 3: ror64(rn, shift_amount); break;
    }
  }
}

// an2823 programming Section 1.7 on addressing modes
int unsignedOffset(int sf, uint64_t *xn, int imm12) {
  if (sf == 0) {
    return *xn + imm12 * 4;
  } 
  return *xn + imm12 * 8;
}

int preIndex(uint64_t *xn, int64_t simm9) {
  *xn += simm9;
  return *xn;
} 

int postIndex(uint64_t *xn, int64_t simm9) {
  uint64_t prevXn = *xn;
  *xn += simm9;
  return prevXn; 
}

int registerOffset(uint64_t *xn, uint64_t *xm) {
  return *xn + *xm;
}

int loadLiteral(uint64_t simm19, uint64_t PC) {
  return PC + (simm19 * 4);
}

int main(int argc, char **argv) {
  // an2823 debugging some of the bitwise shifts programmed
  uint64_t reg1 = 0x765a33f;
  uint64_t reg2 = -1600;
  uint64_t *r1 = &reg1;
  uint64_t *r2 = &reg2;
  int shift_num = 4;

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      bitwiseShift(r2, i, j, shift_num);
      printf("Reg1: %" PRIu64 "\n", reg1);
      printf("Reg1 Hex: 0x%" PRIx64 "\n\n", reg1);
    }
  }

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


// zl4323 Initial Commit