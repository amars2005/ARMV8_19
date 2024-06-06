#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

//useful constants for decoding
#define SF(i)    bits(i,31,31)
#define OPC(i)   bits(i,29,30)
#define M(i)     bits(i,28,28)
#define OP0(i)   bits(i,25,28)
#define OPI(i)   bits(i,23,25)
#define OP(i)    bits(i,5,22)
#define RD(i)    bits(i,0,4)
#define RN(i)    bits(i,5,9)
#define OP2(i)   bits(i,10,21)
#define SH(i)    bits(i,22,22)
#define SHIFT(i) bits(i,22,23)
#define RM(i)    bits(i,16,20)
#define OPR(i)   bits(i,21,24)
#define SH_OP(i) bits(i,10,15)
#define RA(i)    bits(i,10,14)
#define X(i)     bits(i,15,15)
#define BT(i)    bits(i,29,31)
#define SI26(i)  bits(i,0,25)
#define SI19(i)  bits(i,5,23)
#define COND(i)  bits(i,0,3)
#define XN(i)    bits(i,5,9)

#define BRANCH 0
#define BREG   6
#define BCOND  2

// structure representing Processor State Register
typedef struct {
  bool N;
  bool Z;
  bool C;
  bool V;
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

  // calculate length of file
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

///////////////////
//FETCH-DECODE part
///////////////////

// returns the bits [start, end] of i
static uint64_t bits(uint64_t i, int start, int end) {
  return (((i) >> (start)) & (uint32_t) pow(2, (end) - (start) + 1) - 1);
}

static uint32_t fetch(void) {
    uint8_t* ci = state.memory + state.PC; // address of next instruction in memory
    return (ci[0] + (ci[1] << 8) + (ci[2] << 16)); // convert 3 little endian bytes to 32 bit int
}

extern uint64_t bitwiseShift(uint64_t rn, int mode, int instruction, int shift_amount);

#define INAME_SIZE 50

typedef struct {
  bool      sf;
  uint64_t* Rd;
  uint64_t* Rn;
  uint32_t  Op2;
  uint64_t  opc;
} arithmeticDPI;

typedef struct {
  bool      sf;
  uint64_t* Rd;
  uint16_t  Op;
  uint64_t opc;
} wideMoveDPI;

typedef struct {
  uint64_t* Rd;
  uint64_t* Rn;
  uint64_t* Op2;
  uint64_t opc;
} logicDPR;

typedef struct {
  bool      sf;
  bool       X;
  uint64_t* Rd;
  uint64_t* Rn;
  uint64_t* Ra;
} multiplyDPR;

typedef struct {
  int64_t offset;
} branch;

typedef struct {
  uint64_t* Xn;
} breg;

typedef struct {
  int64_t offset;
  uint64_t cond;
} bcond;

typedef union {
    arithmeticDPI arithmeticDpi;
    wideMoveDPI wideMoveDpi;
    logicDPR logicDpr;
    multiplyDPR multiplyDpr;
    branch branch;
    breg breg;
    bcond bcond;
    char itype[INAME_SIZE];
} instruction;

instruction decodeArithmeticDPI(uint32_t i) {
  instruction instr       = { .itype = "arithmeticDPI" };
  instr.arithmeticDpi.sf  = SF(i);
  instr.arithmeticDpi.Rd  = state.R + RD(i);
  instr.arithmeticDpi.Rn  = state.R + RN(i);
  instr.arithmeticDpi.Op2 = OP2(i);
  instr.arithmeticDpi.opc = OPC(i);
  if (SH(i) == 1) { instr.arithmeticDpi.Op2 <<= 12; } // apply sh flag
  return instr;
}

instruction decodeWideMoveDPI(uint32_t i) {
  instruction instr     = { .itype = "wideMoveDPI" };
  instr.wideMoveDpi.Rd  = state.R + RD(i);
  instr.wideMoveDpi.Op  = OP2(i);
  instr.wideMoveDpi.sf  = SF(i);
  instr.wideMoveDpi.opc = OPC(i);
  return instr;
}

instruction decodeArithmeticDPR(uint32_t i) {
  instruction instr;
  return instr;
}

instruction decodeLogicDPR(uint32_t i) {
  instruction instr  = { .itype = "logicDPR" };
  instr.logicDpr.Op2 = state.R + bitwiseShift(RM(i), SF(i), SHIFT(i), SH_OP(i));
  instr.logicDpr.Rd  = state.R + RD(i);
  instr.logicDpr.Rn  = state.R + RN(i); 
  instr.logicDpr.opc = OPC(i);
  //check for 11111 which represets ZR
  if (instr.logicDpr.Op2 == (uint64_t*) 63) { instr.logicDpr.Op2 =  (uint64_t* const) &state.ZR; }
  if (instr.logicDpr.Rn  == (uint64_t*) 63) { instr.logicDpr.Rn  =  (uint64_t* const) &state.ZR; }
  return instr;
}

instruction decodeMultiplyDPR(uint32_t i) {
  instruction instr    = { .itype = "multiplyDPR" };
  instr.multiplyDpr.sf = SF(i);
  instr.multiplyDpr.Rd = state.R + RD(i);
  instr.multiplyDpr.Rn = state.R + RN(i);
  instr.multiplyDpr.Ra = state.R + RA(i);
  instr.multiplyDpr.X  = X(i);
  return instr;
}

instruction decodeBranch(uint32_t i) {
  instruction instr = { .itype = "branch" };
  instr.branch.offset = SI26(i) * 4;
  return instr;
}

instruction decodeBreg(uint32_t i) {
  instruction instr = { .itype = "breg" };
  instr.breg.Xn = (uint64_t*) XN(i);
  return instr;
}

instruction decodeBcond(uint32_t i) {
  instruction instr  = { .itype = "bcond" };
  instr.bcond.offset = SI19(i) * 4;
  instr.bcond.cond   = COND(i);
  return instr;
}

instruction decodeDPI(uint32_t i) {
    uint8_t opi = OPI(i);
    switch (opi) {
        case 2: // opi: 010
            return decodeArithmeticDPI(i);
        case 5: // opi: 101
            return decodeWideMoveDPI(i);
        default:
            fprintf(stderr, "Unsupported operation");
            exit(1);
    };
}

instruction decodeDPR(uint32_t i) {
  uint8_t opr = OPR(i);
  bool    M   = M(i);
  if (M == 0 && (opr & 9) == 8) { return decodeArithmeticDPR(i); } // opr: 1xx0
  if (M == 0 && (opr & 8) == 0) { return decodeLogicDPR(i); }      // opr: 0xxx
  if (M == 1 && opr       == 8) { return decodeMultiplyDPR(i); }   // opr: 1000
  fprintf(stderr, "Unknown operation");
  exit(1);
}

instruction decodeLS(uint32_t i) {}

instruction decodeB(uint32_t i) {
  int branch_t = BT(i);
  switch (branch_t) {
    case(BRANCH):
      return decodeBranch(i);
    case(BREG):
      return decodeBreg(i);
    case(BCOND):
      return decodeBcond(i);
    default:
      //unsupported operation
      fprintf(stderr, "Unknown instruction");
      exit(1);
  }
}

instruction decode(uint32_t i) {
    uint8_t op0 = OP0(i);
    if ((op0 >> 1) == 3) { return decodeDPI(i); } // op0: 100x
    if ((op0 & 7)  == 5) { return decodeDPR(i); } // op0: x101
    if ((op0 & 5)  == 4) { return decodeLS(i);  } // op0: x1x0
    if ((op0 >> 1) == 5) { return decodeB(i);   } // op0: 101x
    fprintf(stderr, "Unknown operation");
    exit(1);
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