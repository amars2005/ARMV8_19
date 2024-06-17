#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>

#define RD(i)      (i)
#define RN(i)      ((i) << 5)
#define RA(i)      ((i) << 10)
#define RM(i)      ((i) << 16)
#define X          ((1) << 15)
#define SF(i)      ((i) << 31)
#define MUL        (27 << 24) // 11011
#define DPI        ( 8 << 26)
#define ARITHM_DPI (2 << 23)
#define SH(i)      ((i) << 22)
#define IMM12(i)   ((i) << 10)
#define OPC(i)     ((i) << 29)
#define HW(i)      ((i) << 21)
#define IMM16(i)   ((i) << 5)

#define movn 0
#define movz 2
#define movk 3

typedef enum { add, adds, sub, subs } arithmeticDPI_t;

uint32_t assembleMultiply(char* opc, int rd, int rn, int rm, int ra, bool sf);
uint32_t assembleArithmeticDPI(char* opc, int rd, int rn, int imm12, bool sh, bool sf);
uint32_t assembleWideMoveDPI(char* opc, int rd, int imm16, int hw, bool sf);