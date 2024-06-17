#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "DPI-assembler.h"

uint32_t assembleMultiply(char* opc, int rd, int rn, int rm, int ra, bool sf) {
    uint32_t instr = SF(sf) + MUL + RM(rm) + RA(ra) + RN(rn) + RD(rd);
    if (strcmp(opc, "msub") == 0) {
        instr += X;
    }
    return instr;
}

