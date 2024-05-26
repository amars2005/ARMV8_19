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

void valueToStr(char valueAsStr[], uint64_t value) {
  sprintf(valueAsStr, "%lx", value); //creates the value as a string
  while (strlen(valueAsStr) < VALUE_STR_LENGTH) {
    strcat("0", valueAsStr); //prefixes the string with the necessary number of 0's
  }
}

void generateLine(uint64_t value, char line[], char outputString[]) {
  char valueAsStr[VALUE_STR_LENGTH]; //creates a line with the register name
  valueToStr(valueAsStr, value);
  strcat(line, valueAsStr); //adds the value to the line
  strcat(line, "\n");
  strcat(outputString, line); //adds the line to the string
}

void outputFile(state cstate, char outputString[]) {
  for (int i; i < GREG_NUM; i++) { //generates the line for the general registers
    uint64_t value = cstate.R[i];
    char line[LINE_STR_LENGTH];
    sprintf(line, "X%d = ", i);
    generateLine(value, line, outputString);
  }
  char pc[] = ("PC = "); //generates the line for the program counter
  uint64_t value = cstate.PC;
  generateLine(value, pc, outputString);  
  char pstate[] = ("PSTATE : "); //generates the line to be outputted for pstate
  if (cstate.PSTATE.Z) { strcat(pstate, "Z"); } else { strcat(pstate, "-"); }
  if (cstate.PSTATE.C) { strcat(pstate, "C"); } else { strcat(pstate, "-"); }
  if (cstate.PSTATE.N) { strcat(pstate, "N"); } else { strcat(pstate, "-"); }
  if (cstate.PSTATE.V) { strcat(pstate, "V"); } else { strcat(pstate, "-"); }
  strcat(outputString, pstate);
  for (int i = 0; i < MEM_SIZE; i++) { //checks non-zero memory and adds it to the output string
    if (cstate.memory[i] != 0) {
      char line[LINE_STR_LENGTH];
      sprintf(line, "%d = ", i);
      generateLine(cstate.memory[i], line, outputString);
    }
  }
}

int main(int argc, char **argv) {
  state cstate = { .ZR = 0 }; // initializes the machine
  setup(&cstate);
  return EXIT_SUCCESS;
}