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

uint32_t assembleArithmeticDPI(char* opc, int rd, int rn, int imm12, bool sh, bool sf) {
    uint32_t instr = SF(sf) + DPI + ARITHM_DPI + SH(sh) + IMM12(imm12) + RN(rn) + RD(rd);
    if (strcmp(opc, "add") == 0)  { instr += OPC(add); }
    if (strcmp(opc, "adds") == 0) { instr += OPC(adds); }
    if (strcmp(opc, "sub") == 0)  { instr += OPC(sub); }
    if (strcmp(opc, "subs") == 0) { instr += OPC(subs); }
    return instr;
}

uint32_t assembleWideMoveDPI(char* opc, int rd, int imm16, int hw, bool sf) {
    uint32_t instr = SF(sf) + DPI + ARITHM_DPI + HW(hw) + IMM16(imm16) + RD(rd);
    if (strcmp(opc, "movn") == 0)  { instr += OPC(movn); }
    if (strcmp(opc, "movz") == 0)  { instr += OPC(movz); }
    if (strcmp(opc, "movk") == 0)  { instr += OPC(movk); }
    return instr;
}