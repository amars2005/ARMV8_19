#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include "instruction-types.h"

#define RD(i)      (i)
#define RN(i)      ((i) << 5)
#define RA(i)      ((i) << 10)
#define RM(i)      ((i) << 16)
#define X_BIT      ((1) << 15)
#define SF(i)      ((i) << 31)
#define MUL        (27 << 24) // 11011
#define DPI        (4 << 26)
#define ARITHM_DPI (2 << 23)
#define SH(i)      ((i) << 22)
#define IMM12(i)   ((i) << 10)
#define OPC(i)     ((i) << 29)
#define HW(i)      ((i) << 21)
#define IMM16(i)   ((i) << 5)
#define WMOVE_DPI  ((5) << 23)
#define ARITHM_DPR ( 1  << 24)
#define DPR        ( 5  << 25)
#define SHIFT(i)   ((i) << 22)
#define LOGIC_DPR  ( 0 )
#define N_BIT      ( 1 << 21)
#define OP2(i)     ((i) << 10)
#define OPERAND(i) ((i) << 10)

#define IMM12_SIZE ((1 << 12) - 1)

#define movn 0
#define movz 2
#define movk 3

#define LITTLE(i)  ((i & 0xFF) << 24) | \
                   ((i & 0xFF00) << 8) | \
                   ((i >> 8) & 0xFF00) | \
                   (i >> 24);

extern uint32_t assembleMultiply(bool x, uint64_t rd, uint64_t rn, uint64_t rm, uint64_t ra, bool sf);
extern uint32_t assembleArithmeticDPI(arithmeticDPI_t opc, uint64_t rd, uint64_t rn, uint64_t imm12, bool sh, bool sf);
extern uint32_t assembleWideMoveDPI(uint64_t opc, uint64_t rd, uint64_t imm16, uint64_t hw, bool sf);
extern uint32_t assembleArithmeticDPR(arithmeticDPI_t opc, uint64_t rd, uint64_t rn, uint64_t rm, uint64_t shift, bool sf);
extern uint32_t assembleLogicDPR(uint64_t opc, uint64_t rd, uint64_t rn, uint64_t rm, uint64_t shift, uint64_t operand, bool n, bool sf);