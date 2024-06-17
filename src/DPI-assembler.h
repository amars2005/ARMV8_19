#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>

#define RD(i) (i)
#define RN(i) ((i) << 5)
#define RA(i) ((i) << 10)
#define RM(i) ((i) << 16)
#define X     ((1) << 15)
#define SF(i) ((i) << 31)
#define MUL    (27 << 24) // 11011

uint32_t assembleMultiply(char* opc, int rd, int rn, int rm, int ra, bool sf);
