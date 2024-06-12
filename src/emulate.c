#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <stdbool.h>

/*
call example: ./emulate <file_in>            - output to stdout
         alt: ./emulate <file_in> <file_out> - output to file
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
#define L(i)     bits(i,22,22)
#define U(i)     bits(i,24,24)
#define SFt(i)   bits(i,30,30)
#define HW(i)    bits(i,21,22)
#define IMM16(i) bits(i,5,20)

#define BRANCH 0
#define BREG   6
#define BCOND  2

#define HALT   0x8a000000

typedef enum { arithmeticDPIt, wideMoveDPIt, arithmeticDPRt, logicDPRt, multiplyDPRt, brancht, bregt, bcondt, sdt, ll } instruction_t;
typedef enum { add, adds, sub, subs } arithmeticDPI_t;
typedef enum { and, orr, eor, ands} logicDPR_t;
typedef enum { bic, orn, eon, bics} logicDPRN_t;

#define movn 0
#define movz 2
#define movk 3

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

// returns the bits [start, end] of i
static uint64_t bits(uint64_t i, int start, int end) {
    return (((i) >> (start)) & ((uint32_t) pow(2, (end) - (start) + 1) - 1));
}

void generateLine(uint64_t value, char line[], char outputString[]) {
  char x[17];
  sprintf(x, "%016" PRIx64, value);
  strcat(line, x); //adds the value to the line
  strcat(line, "\n");
  strcat(outputString, line); //adds the line to the string
}

void nonZeroGenerateLine(int i, char* line) {
  char number[3];
  sprintf(number, "%02x", state.memory[i]);
  strcat(line, number);
}

void outputFile(char outputString[]) {
  for (int i = 0; i < GREG_NUM; i++) { //generates the line for the general registers
    uint64_t value = state.R[i];
    char line[LINE_STR_LENGTH];
    if (i < 10) {
      sprintf(line, "X0%d = ", i);
    } else {
      sprintf(line, "X%d = ", i);
    }
    generateLine(value, line, outputString);
  }

  char pc[] = "PC = "; //generates the line for the program counter
  uint64_t value = state.PC;
  generateLine(value, pc, outputString);  
  char pstate[] = "PSTATE : "; //generates the line to be outputted for pstate

  if (state.PSTATE.N==1) { strcat(pstate, "N"); } else { strcat(pstate, "-"); }
  if (state.PSTATE.Z==1) { strcat(pstate, "Z"); } else { strcat(pstate, "-"); }
  if (state.PSTATE.C==1) { strcat(pstate, "C"); } else { strcat(pstate, "-"); }
  if (state.PSTATE.V==1) { strcat(pstate, "V\n"); } else { strcat(pstate, "-\n"); }
  strcat(outputString, pstate);

  for (int i = 0; i < MEM_SIZE; i+=4) { //checks non-zero memory and adds it to the output string
    if (state.memory[i] != 0 || state.memory[i+1] != 0 || state.memory[i+2] != 0 || state.memory[i+3] != 0) {
      char line[LINE_STR_LENGTH];
      sprintf(line, "0x%08X : ", i);
      nonZeroGenerateLine(i+3, line);
      nonZeroGenerateLine(i+2, line);
      nonZeroGenerateLine(i+1, line);
      nonZeroGenerateLine(i, line);
      strcat(line, "\n");
      strcat(outputString, line); //adds any to the output
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

// Functions for data processing instructions using immediate addressing (1.4)

void immAdd(uint64_t *rd, const uint64_t *rn, const uint64_t *imm12, bool z) {
  // Bitwise ADD on the values pointed to by rn and imm12
  *rd = *rn + *imm12;
} 

void immAddFlags(uint64_t *rd, const uint64_t *rn, const uint64_t *imm12, bool z) {
  // Bitwise ADD on the values pointed to by rn and imm12
  uint64_t result = *rn + *imm12;
  *rd = result;

  int bitNum;
  if (!z) { bitNum = 32; }
  else { bitNum = 64; }

  (state.PSTATE).N = (result >> (bitNum - 1));
  (state.PSTATE).Z = (result == 0); 
  (state.PSTATE).C = (result > (1ULL << bitNum)); 
  (state.PSTATE).V = (result > (1ULL << (bitNum - 1))); 
}

void immSub(uint64_t *rd, const uint64_t *rn, const uint64_t *imm12, bool z) {
  // Bitwise SUB on the values pointed to by rn and imm12
  *rd = *rn - *imm12;
} 

void immSubFlags(uint64_t *rd, const uint64_t *rn, const uint64_t *imm12, bool z) {
  // Bitwise SUB on the values pointed to by rn and imm12
  uint64_t result = *rn - *imm12;
  *rd = result;
  
  int bitNum;
  if (!z) { bitNum = 32; }
  else { bitNum = 64; }

  (state.PSTATE).N = (result >> (bitNum - 1));
  (state.PSTATE).Z = (result == 0); 
  (state.PSTATE).C = !(state.PSTATE).N; 
  (state.PSTATE).V = (*rn > *imm12) && (*rd < *imm12);
}

void wMovN(uint64_t *rd, const uint64_t *hw, const uint64_t *imm16, bool z) {
  //Sets the value in rd to the bitwise negation of imm16
  int bits;
  if (!z) { bits = 32; }
  else { bits = 64; }

  *rd = (2 << bits) + (*imm16) - (2 << (*hw));
}

void wMovZ(uint64_t *rd, const uint64_t *hw, const uint64_t *imm16, bool z) {
  //Sets the value in rd to imm16
  *rd = *imm16;
}

void wMovK(uint64_t *rd, const uint64_t *hw, const uint64_t *imm16, bool z) {
  //Inserts the value of imm16 into rd, keeping all the other bits the same.
//  int bits;
//  if (!z) { bits = 32; }
//  else { bits = 64; }

  *rd = *rd - bits(*rd, *hw, *hw + 15) + (*imm16 * 2 << *hw);
}

// Functions for data processing instructions with registers (1.5)

void regAnd(uint64_t *rd, const uint64_t *rn, const uint64_t *op2) {
  // Bitwise AND on the values pointed to by rn and op2
  *rd = *rn & *op2;
} 

void regClear(uint64_t *rd, const uint64_t *rn, const uint64_t *op2) {
  // BIC is the same as AND with the complement of the second operand
  *rd = *rn & ~(*op2);
}

void regOr(uint64_t *rd, const uint64_t *rn, const uint64_t *op2) {
  // Bitwise OR on the values pointed to by rn and op2
  *rd = *rn | *op2;
}

void regOrn(uint64_t *rd, const uint64_t *rn, const uint64_t *op2) {
  // Bitwise OR with the complement of the second operand
  *rd = *rn | ~(*op2);
}

void regXor(uint64_t *rd, const uint64_t *rn, const uint64_t *op2) {
  // Bitwise XOR on the values pointed to by rn and op2
  *rd = *rn ^ *op2;
}

void regXorn(uint64_t *rd, const uint64_t *rn, const uint64_t *op2) {
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

void regAndFlags(uint64_t *rd, const uint64_t *rn, const uint64_t *op2) {
  // Bitwise AND on the values pointed to by rn and op2
  uint64_t result = *rn & *op2;
  *rd = result;
  updateFlags(result);
} 

void regClearFlags(uint64_t *rd, const uint64_t *rn, const uint64_t *op2) {
  // Bitwise BIC on rn and op2
  uint64_t result = *rn & ~(*op2);
  *rd = result;
  updateFlags(result);
} 

void regmAdd(uint64_t *rd, const uint64_t *ra, const uint64_t *rn, const uint64_t *rm) {
  // Perform an mAdd on the values stored in ra, rn and rm
  *rd = *ra + ((*rn) * (*rm));
}

void regmSub(uint64_t *rd, const uint64_t *ra, const uint64_t *rn, const uint64_t *rm) {
  // Perform an mSub on the values stored in ra, rn and rm
  *rd = *ra - ((*rn) * (*rm));
}


// Functions relating to bitwise shifts (1.6)

uint64_t lsl32(uint64_t rn, int shift_amount) {
  uint32_t lowerBits = (uint32_t) (rn & 0xFFFFFFFF);
  return (uint64_t) (lowerBits << shift_amount);
}

uint64_t lsr32(uint64_t rn, int shift_amount) {
  uint32_t lowerBits = (uint32_t) (rn & 0xFFFFFFFF);
  return (uint64_t) (lowerBits >> shift_amount);
}

uint64_t asr32(uint64_t rn, int shift_amount) {
  uint32_t lowerBits = (uint32_t) (rn & 0xFFFFFFFF);
  uint32_t sign = lowerBits & 0x80000000;
  for (int i = 0; i < shift_amount; i++) {
    lowerBits >>= 1;
    lowerBits |= sign;
  }
  return (uint64_t) lowerBits;
}

uint64_t ror32(uint64_t rn, int shift_amount) {
  uint32_t lowerBits = (uint32_t) (rn & 0xFFFFFFFF);
  for (int i = 0; i < shift_amount; i++) {
    uint32_t lsb = lowerBits & 0x00000001;
    lowerBits >>= 1;
    if (lsb != 0) {
      lowerBits |= 0x80000000;
    } else {
      lowerBits &= 0x7FFFFFFF;
    }
  }
  return lowerBits;
}

uint64_t lsl64(uint64_t rn, int shift_amount) {
  return (rn << shift_amount);
}

uint64_t lsr64(uint64_t rn, int shift_amount) {
  return (rn >> shift_amount);
}

uint64_t asr64(uint64_t rn, int shift_amount) {
  uint64_t sign = rn & 0x8000000000000000;
  for (int i = 0; i < shift_amount; i++) {
    rn >>= 1;
    rn = rn | sign;
  }
  return rn;
}

uint64_t ror64(uint64_t rn, int shift_amount) {
  for (int i = 0; i < shift_amount; i++) {
    uint64_t lsb = rn & 0x0000000000000001;
    rn >>= 1;
    if (lsb != 0) {
      rn |= 0x8000000000000000;
    } else {
      rn &= 0x7FFFFFFFFFFFFFFF;
    }
  } 
  return rn;
}


// First input is the register number
// Second input is the register mode (32 or 64 bit) represented by 0 and 1 respectively
// Third input is the instruction type (lsl, lsr, asr, ror)
// 0, 1, 2, 3 for lsl, lsr, asr, ror respectively
uint64_t bitwiseShift(uint64_t rn, int mode, int instruction, int shift_amount) {
  assert(instruction >= 0 && instruction <= 3);

  assert(mode == 0 || mode == 1);

  if (mode == 0) {
    switch (instruction) {
      case 0: return lsl32(rn, shift_amount);
      case 1: return lsr32(rn, shift_amount);
      case 2: return asr32(rn, shift_amount);
      case 3: return ror32(rn, shift_amount);
      default: exit(1);
    }
  } else {
    switch (instruction) {
      case 0: return lsl64(rn, shift_amount); 
      case 1: return lsr64(rn, shift_amount); 
      case 2: return asr64(rn, shift_amount); 
      case 3: return ror64(rn, shift_amount);
        default: exit(1);
    }
  }
}

// Section 1.7 on addressing modes

// Begin with helper function on loading and storing
void load(uint64_t *rn, uint8_t sf, uint64_t addr) {
  uint64_t regVal = 0;
  if (sf == 0) {
    for (int i = 0; i < 4; i++) {
      regVal += state.memory[addr + i] << (i * 8);
    }
    *rn = regVal;
  } else {
    for (int i = 0; i < 8; i++) {
      regVal += state.memory[addr + i] << (i * 8);
    }
    *rn = regVal;
  }
}

void store(uint64_t *rn, uint8_t sf, uint64_t addr) {
  uint64_t twoBitMask = 0xFF;
  if (sf == 0) {
    for (int i = 0; i < 4; i++) {
      state.memory[addr + i] = (uint8_t) ((*rn >> (i * 8)) & twoBitMask);
    }
  } else {
    for (int i = 0; i < 8; i++) {
      state.memory[addr + i] = (uint8_t) ((*rn >> (i * 8)) & twoBitMask);
    }
  }
  load(rn, sf, addr);
}

void unsignedOffset(uint8_t sf, uint64_t *xn, uint64_t imm12, uint8_t L, uint64_t *rt) {
  uint64_t uoffset = imm12 << (2 + sf);
  if (L == 1) {
    load(rt, sf, *xn + uoffset);
  } else {
    store(rt, sf, *xn + uoffset);
  }
}

void preIndex(uint8_t sf, uint64_t *xn, int64_t simm9, uint8_t L, uint64_t *rt) {
  uint64_t transferAddr = *xn + simm9;
  if (L == 1) {
    load(rt, sf, transferAddr);
  } else {
    store(rt, sf, transferAddr);
  }
  *xn += simm9;
} 

void postIndex(uint8_t sf, uint64_t *xn, int64_t simm9, uint8_t L, uint64_t *rt) {
  if (L == 1) {
    load(rt, sf, *xn);
  } else {
    store(rt, sf, *xn);
  }
  *xn += simm9;
}

void registerOffset(uint8_t sf, uint64_t *xn, uint64_t *xm, uint8_t L, uint64_t *rt) {
  if (L == 1) {
    load(rt, sf, *xn + *xm);
  } else {
    store(rt, sf, *xn + *xm);
  }
}


void singleDataTransfer(uint8_t sf, uint8_t U, uint8_t L, uint64_t offset, uint64_t *xn, uint64_t *rt) {
  if (U == 1) {
    unsignedOffset(sf, xn, offset, L, rt);
  } else {
    uint64_t msb = offset >> 11;

    if (msb == 0) {
      uint64_t I = (offset >> 1) & 1;
      uint64_t simm9 = (offset >> 2) & 0x1FF;

      if (I == 0) {
        postIndex(sf, xn, simm9, L, rt);
      } else {
        preIndex(sf, xn, simm9, L, rt);
      }
    } else {
      uint64_t regNum = (offset >> 6) & 0x1F;
      uint64_t *xm = &(state.R[regNum]);
      registerOffset(sf, xn, xm, L, rt);
    }
  }
}

void loadLiteral(uint8_t sf, uint64_t simm19, uint64_t *rt) {
  uint64_t transferAddr = state.PC + (simm19 << 2);
  load(rt, sf, transferAddr);
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

// Function for halt 1.9

int halt() {
  exit(0);
}

///////////////////
//FETCH-DECODE part
///////////////////

static uint32_t fetch(void) {
    uint8_t* ci = state.memory + state.PC; // address of next instruction in memory
    return (ci[0] + (ci[1] << 8) + (ci[2] << 16) + (ci[3] << 24)); // convert 4 little endian bytes to 32 bit int
}

// Structs representing different instruction types
typedef struct {
  bool      sf;
  uint64_t* Rd;
  uint64_t* Rn;
  uint64_t  Op2;
  uint64_t  opc;
} arithmeticDPI;

typedef struct {
  bool      sf;
  uint64_t  hw;
  uint64_t* Rd;
  uint64_t  Op;
  uint64_t opc;
} wideMoveDPI;

typedef struct {
  bool      sf;
  uint64_t* Rd;
  uint64_t* Rn;
  uint64_t* Rm;
  uint64_t  opc;
} arithmeticDPR;

typedef struct {
  bool      sf;
  uint64_t* Rd;
  uint64_t* Rn;
  uint64_t* Op2;
  uint64_t opc;
  bool N;
} logicDPR;

typedef struct {
  bool      sf;
  bool       X;
  uint64_t* Rd;
  uint64_t* Rn;
  uint64_t* Ra;
  uint64_t* Rm;
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

typedef struct {
  bool sf;
  bool u;
  bool l;
  uint32_t offset;
  uint64_t* Xn;
  uint64_t* Rt;
} SDT;

typedef struct {
  bool sf;
  uint32_t simm19;
  uint64_t* Rt;
} LL;

typedef union {
    instruction_t itype;
    arithmeticDPI arithmeticDpi;
    wideMoveDPI wideMoveDpi;
    arithmeticDPR arithmeticDpr;
    logicDPR logicDpr;
    multiplyDPR multiplyDpr;
    branch branch;
    breg breg;
    bcond bcond;
    SDT sdt;
    LL ll;
} instrData;

typedef struct {
  instrData     instruction;
  instruction_t itype;
} instruction;

// Function decoding each instruction type
instruction decodeArithmeticDPI(uint32_t i) {
  instruction instr       = { .itype = arithmeticDPIt };
  instr.instruction.arithmeticDpi.sf  = SF(i);
  instr.instruction.arithmeticDpi.Rd  = state.R + RD(i);
  instr.instruction.arithmeticDpi.Rn  = state.R + RN(i);
  instr.instruction.arithmeticDpi.Op2 = OP2(i);
  instr.instruction.arithmeticDpi.opc = OPC(i);
  if (SH(i) == 1) { instr.instruction.arithmeticDpi.Op2 <<= 12; } // apply sh flag
  return instr;
}

instruction decodeWideMoveDPI(uint32_t i) {
  instruction instr     = { .itype = wideMoveDPIt };
  instr.instruction.wideMoveDpi.Rd  = state.R + RD(i);
  instr.instruction.wideMoveDpi.hw  = HW(i);
  instr.instruction.wideMoveDpi.Op  = IMM16(i);
  instr.instruction.wideMoveDpi.sf  = SF(i);
  instr.instruction.wideMoveDpi.opc = OPC(i);
  return instr;
}

instruction decodeArithmeticDPR(uint32_t i) {
  instruction instr = { .itype = arithmeticDPRt };
  instr.instruction.arithmeticDpr.sf = SF(i);
  instr.instruction.arithmeticDpr.Rd = state.R + RD(i);
  instr.instruction.arithmeticDpr.Rn = state.R + RN(i);
  instr.instruction.arithmeticDpr.Rm = state.R + RM(i);
  instr.instruction.arithmeticDpr.opc = OPC(i);
  instr.itype = arithmeticDPRt;
  return instr;
}

instruction decodeLogicDPR(uint32_t i) {
  instruction instr  = { .itype = logicDPRt };
  instr.instruction.logicDpr.sf  = SF(i);
  instr.instruction.logicDpr.Op2 = state.R + bitwiseShift(RM(i), SF(i), SHIFT(i), SH_OP(i));
  instr.instruction.logicDpr.Rd  = state.R + RD(i);
  instr.instruction.logicDpr.Rn  = state.R + RN(i); 
  instr.instruction.logicDpr.opc = OPC(i);
  //check for 11111 which represents ZR
  if (instr.instruction.logicDpr.Op2 == (uint64_t*) 63) { instr.instruction.logicDpr.Op2 =  (uint64_t* const) &state.ZR; }
  if (instr.instruction.logicDpr.Rn  == (uint64_t*) 63) { instr.instruction.logicDpr.Rn  =  (uint64_t* const) &state.ZR; }
  return instr;
}

instruction decodeMultiplyDPR(uint32_t i) {
  instruction instr    = { .itype = multiplyDPRt };
  instr.instruction.multiplyDpr.sf = SF(i);
  instr.instruction.multiplyDpr.Rd = state.R + RD(i);
  instr.instruction.multiplyDpr.Rn = state.R + RN(i);
  instr.instruction.multiplyDpr.Ra = state.R + RA(i);
  instr.instruction.multiplyDpr.Rm = state.R + RM(i);
  instr.instruction.multiplyDpr.X  = X(i);
  return instr;
}

instruction decodeBranch(uint32_t i) {
  instruction instr = { .itype = brancht };
  instr.instruction.branch.offset = SI26(i) * 4;
  return instr;
}

instruction decodeBreg(uint32_t i) {
  instruction instr = { .itype = bregt };
  instr.instruction.breg.Xn = (uint64_t*) XN(i);
  return instr;
}

instruction decodeBcond(uint32_t i) {
  instruction instr  = { .itype = bcondt };
  instr.instruction.bcond.offset = SI19(i) * 4;
  instr.instruction.bcond.cond   = COND(i);
  return instr;
}

instruction decodeSDT(uint32_t i) {
  instruction instr = { .itype = sdt };
  instr.instruction.sdt.sf = SFt(i);
  instr.instruction.sdt.u  = U(i);
  instr.instruction.sdt.l  = L(i);
  instr.instruction.sdt.offset = OP2(i);
  instr.instruction.sdt.Xn = state.R + XN(i);
  instr.instruction.sdt.Rt = state.R + RD(i);
  return instr;
}

instruction decodeLL(uint32_t i) {
  instruction instr = { .itype = ll };
  instr.instruction.ll.sf = SFt(i);
  instr.instruction.ll.Rt = state.R + RD(i);
  instr.instruction.ll.simm19 = SI19(i);
  return instr;
}

// Decode flow functions
instruction decodeDPI(uint32_t i) {
    uint8_t opi = OPI(i);
    switch (opi) {
        case 2: // opi: 010
            return decodeArithmeticDPI(i);
        case 5: // opi: 101
            return decodeWideMoveDPI(i);
        default:
            fprintf(stderr, "Unsupported operation in DPI");
            exit(1);
    };
}

instruction decodeDPR(uint32_t i) {
  uint8_t opr = OPR(i);
  bool    M   = M(i);
  if (M == 0 && (opr & 9) == 8) { return decodeArithmeticDPR(i); } // opr: 1xx0
  if (M == 0 && (opr & 8) == 0) { return decodeLogicDPR(i); }      // opr: 0xxx
  if (M == 1 && opr       == 8) { return decodeMultiplyDPR(i); }   // opr: 1000
  fprintf(stderr, "Unknown operation in DPR");
  exit(1);
}

instruction decodeLS(uint32_t i) {
  if (SF(i)) {
    return decodeSDT(i);
  } else {
    return decodeLL(i);
  }
}

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

// Main decode function
instruction decode(uint32_t i) {
    uint8_t op0 = OP0(i);
    if ((op0 >> 1) == 4) { return decodeDPI(i); } // op0: 100x
    if ((op0 & 7)  == 5) { return decodeDPR(i); } // op0: x101
    if ((op0 & 5)  == 4) { return decodeLS(i);  } // op0: x1x0
    if ((op0 >> 1) == 5) { return decodeB(i);   } // op0: 101x
    else {
        fprintf(stderr, "Unknown operation in decode: op0 is %d, i is %d", op0, i);
        exit(1);
    }
}

////////////////
//EXECUTION PART
////////////////

void executeArithmeticDPI(instruction i) {
    void (*func)(uint64_t*, const uint64_t*, const uint64_t*, bool);
  switch (i.instruction.arithmeticDpi.opc) {
    case (add):
        func = &immAdd;
        break;
    case (adds):
        func = &immAddFlags;
        break;
    case (sub):
        func = &immSub;
        break;
    case (subs):
        func = &immSubFlags;
        break;
  }
    (*func)(i.instruction.arithmeticDpi.Rd, i.instruction.arithmeticDpi.Rn, &i.instruction.arithmeticDpi.Op2, i.instruction.arithmeticDpi.sf);
}

void executeWideMoveDPI(instruction i) {
    void (*func)(uint64_t *rd, const uint64_t *hw, const uint64_t *imm16, const bool z);
  switch (i.instruction.wideMoveDpi.opc) {
    case (movn):
        func = &wMovN;
        break;
    case (movz):
        func = &wMovZ;
        break;
    case (movk):
        func = &wMovK;
        break;
  }
    (*func)(i.instruction.wideMoveDpi.Rd, &i.instruction.wideMoveDpi.hw, &i.instruction.wideMoveDpi.Op, i.instruction.arithmeticDpi.sf);
}

void executeArithmeticDPR(instruction i) {
  void (*func)(uint64_t*, const uint64_t*, const uint64_t*, bool);
  switch (i.instruction.arithmeticDpr.opc) {
    case (add):
        func = &immAdd;
        break;
    case (adds):
        func = &immAddFlags;
        break;
    case (sub):
        func = &immSub;
        break;
    case (subs):
        func = &immSubFlags;
        break;
  }
    (*func)(i.instruction.arithmeticDpr.Rd, i.instruction.arithmeticDpr.Rn, i.instruction.arithmeticDpr.Rm, i.instruction.arithmeticDpi.sf);
}

void executeLogicDPR(instruction i) {
    void (*func)(uint64_t *rd, const uint64_t *rn, const uint64_t *op2);
  if (i.instruction.logicDpr.N) {
    switch (i.instruction.logicDpr.opc) {
      case (bic):
          func = &regClear;
        break;
      case (orn):
          func = &regOrn;
        break;
      case (eon):
          func = &regXorn;
        break;
      case (bics):
          func = &regClearFlags;
        break;
    }
  } else {
    switch (i.instruction.logicDpr.opc) {
      case (and):
          func = &regAnd;
        break;
      case (orr):
          func = &regOr;
        break;
      case (eor):
          func = &regXor;
        break;
      case (ands):
          func = &regAndFlags;
        break;
    }
  }
    (*func)(i.instruction.logicDpr.Rd, i.instruction.logicDpr.Rn, i.instruction.logicDpr.Op2);
}

void executeMultiplyDPR(instruction i) {
    void (*func)(uint64_t *rd, const uint64_t *ra, const uint64_t *rn, const uint64_t *rm);
  if (i.instruction.multiplyDpr.X) {
    func = &regmSub;
  } else {
    func = &regmAdd;
  }
    (*func)(i.instruction.multiplyDpr.Rd, i.instruction.multiplyDpr.Ra, i.instruction.multiplyDpr.Rn, i.instruction.multiplyDpr.Rm);
}

void executeBranch(instruction i) {
    unCondBranch(i.instruction.branch.offset);
}

void executeBreg(instruction i) {
    registerBranch(i.instruction.breg.Xn);
}

void executeBcond(instruction i) {
    condBranch(i.instruction.bcond.offset, i.instruction.bcond.cond);
}

void executeSDT(instruction i) {
    singleDataTransfer(i.instruction.sdt.sf, i.instruction.sdt.u, i.instruction.sdt.l, i.instruction.sdt.offset, i.instruction.sdt.Xn, i.instruction.sdt.Rt);
}

void executeLL(instruction i) {
    loadLiteral(i.instruction.ll.sf, i.instruction.ll.simm19, i.instruction.ll.Rt);
}

void execute(instruction i) {
  switch (i.itype) {
    case (arithmeticDPIt):
      executeArithmeticDPI(i);
      break;
    case (wideMoveDPIt):
      executeWideMoveDPI(i);
      break;
    case (arithmeticDPRt):
      executeArithmeticDPR(i);
      break;
    case (logicDPRt):
      executeLogicDPR(i);
      break;
    case (multiplyDPRt):
      executeMultiplyDPR(i);
          break;
    case (brancht):
      executeBranch(i);
          break;
    case (bregt):
      executeBreg(i);
          break;
    case (bcondt):
      executeBcond(i);
          break;
    case (sdt):
      executeSDT(i);
          break;
    case (ll):
      executeLL(i);
          break;
  }
  if (i.itype != brancht && i.itype != bcondt && i.itype != bregt) {
      state.PC += 4;
  }
}

int main(int argc, char **argv) {
  // validate input arguments
  if (argc > 3 || argc < 2) { 
    fprintf(stderr, "Usage: emulate <file_in> [<file_out>]\n");
    exit(1);
  }

  setup();

  loadfile(argv[1]);

  uint32_t i = fetch();
  instruction d;
  while (i != HALT) {
      d = decode(i);
      execute(d);
      i = fetch();
  }

  FILE* out;
  if (argc == 3) {
      out = fopen(argv[2], "w");
      if (out == NULL) {
          fprintf(stdout, "Output file not found\n");
          exit(1);
      }
  } else {
      out = stdout;
  }
  
  char outstr[10000];
  outputFile(outstr);
  fprintf(out, "%s", outstr);
  fclose(out);

  return EXIT_SUCCESS;
}
