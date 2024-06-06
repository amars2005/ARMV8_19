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
} state;

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

uint32_t fetch(void) {
    uint8_t* ci = state.memory + state.PC; // address of next instruction in memory
    return (ci[0] + (ci[1] << 8) + (ci[2] << 16)); // convert 3 little endian bytes to 32 bit int
}

typedef struct {
  uint64_t* Rd;
  uint64_t* Rn;
  uint32_t  Op2;
  char iname[50];
} arithmeticDPI;

typedef struct {
  uint64_t* Rd;
  uint16_t  Op;
  char iname[50];
} wideMoveDPI;

typedef struct {} arithmeticAndLogicDPR;
typedef struct {} multiplyDPR;
typedef struct {} bitwiseShift;

typedef union {
    arithmeticDPI arithmeticDpi;
    wideMoveDPI wideMoveDpi;
    arithmeticAndLogicDPR arithmeticAndLogicDpr;
    multiplyDPR multiplyDpr;
    bitwiseShift bitwiseShift1;
    char itype[50];
} instruction;

instruction decodeArithmeticDPI(uint32_t i) {
    i &= (uint32_t) (pow(2, 22) - 1);                // bits [0,22]
    instruction instr       = { .itype = "arithmeticDPI" };
    strcpy(instr.arithmeticDpi.iname, "arithmeticDPI");
    instr.arithmeticDpi.Rd  = state.R + (i & 15);         // bits [0, 4]
    instr.arithmeticDpi.Rn  = state.R + ((i >> 5) & 31);  // bits [5,9]
    instr.arithmeticDpi.Op2 = (i >> 10) & 4095;           // bits [10,21]
    if (i >> 22 == 1) { instr.arithmeticDpi.Op2 <<= 12; } // apply sh flag
    return instr;
}

instruction decodeWideMoveDPI(uint32_t i) {
  i &= (uint32_t) (pow(2, 22) - 1);     // bits [0,22]
  instruction instr    = { .itype = "wideMoveDPI" };
  strcpy(instr.wideMoveDpi.iname, "wideMoveDPI");
  instr.wideMoveDpi.Rd = state.R + (i & 15); // bits [0, 4]
  instr.wideMoveDpi.Op = (i >> 10) & 4095;   // bits [10,21]
  return instr;
}

instruction decodeDPI(uint32_t i) {
    uint8_t opi = (i >> 23) & 7; // get bits [25, 23]
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
instruction decodeDPR(uint32_t i) {}
instruction decodeLS(uint32_t i) {}
instruction decodeB(uint32_t i) {}

instruction decode(uint32_t i) {
    uint8_t op0 = (i >> 26) & 0xff; // get bits [25,28]
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