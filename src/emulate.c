#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
static void setup(state cstate) {
  for (int i; i < MEM_SIZE; i++) { cstate.memory[i] = 0; }
  for (int i; i < GREG_NUM; i++) { cstate.R[i]      = 0; }
  cstate.PC       = 0;
  cstate.PSTATE.Z = 1;
  cstate.PSTATE.C = 0;
  cstate.PSTATE.N = 0;
  cstate.PSTATE.V = 0;
}

int main(int argc, char **argv) {
  state cstate = { .ZR = 0 }; // initializes the machine
  setup(cstate);

  return EXIT_SUCCESS;
}
