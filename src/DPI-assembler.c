#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "DPI-assembler.h"

uint32_t assembleMultiply(char* opc, int rd, int rn, int rm, int ra, bool sf) {
    uint32_t instr = SF(sf) + MUL + RM(rm) + RA(ra) + RN(rn) + RD(rd);
    if (strcmp(opc, "msub") == 0) {
        instr += X;
    }
    return instr;
}

uint32_t assembleArithmeticDPI(char* opc, int rd, int rn, int imm12, bool sf) {
    bool sh = false;
    if (imm12 > IMM12_SIZE) {
        imm12 >>= 12;
        sh = true;
    }
    uint32_t instr = SF(sf) + DPI + ARITHM_DPI + SH(sh) + IMM12(imm12) + RN(rn) + RD(rd);
    if (strcmp(opc, "add") == 0)  { instr += OPC(add); }
    if (strcmp(opc, "adds") == 0) { instr += OPC(adds); }
    if (strcmp(opc, "sub") == 0)  { instr += OPC(sub); }
    if (strcmp(opc, "subs") == 0) { instr += OPC(subs); }
    return instr;
}

uint32_t assembleWideMoveDPI(char* opc, int rd, int imm16, bool sf) {
    int hw = 0;
    int mask = (1 << 16) - 1;
    while ((imm16 & mask) == 0) {
        hw ++;
        imm16 >>= 16;
    }
    assert(hw < 4);
    uint32_t instr = SF(sf) + DPI + WMOVE_DPI + HW(hw) + IMM16(imm16) + RD(rd);
    if (strcmp(opc, "movn") == 0)  { instr += OPC(movn); }
    if (strcmp(opc, "movz") == 0)  { instr += OPC(movz); }
    if (strcmp(opc, "movk") == 0)  { instr += OPC(movk); }
    return instr;
}

uint32_t assembleArithmeticDPR(char* opc, int rd, int rn, int rm, int shift, int op2, bool sf) {
    uint32_t instr = SF(sf) + DPR + ARITHM_DPR + SHIFT(shift) + RM(rm) + OP2(op2) + RN(rn) + RD(rd);
    if (strcmp(opc, "add") == 0)  { instr += OPC(add); }
    if (strcmp(opc, "adds") == 0) { instr += OPC(adds); }
    if (strcmp(opc, "sub") == 0)  { instr += OPC(sub); }
    if (strcmp(opc, "subs") == 0) { instr += OPC(subs); }
    return instr;
}

uint32_t assembleLogicDPR(char* opc, int rd, int rn, int rm, int shift, int op2, bool sf) {
    uint32_t instr = SF(sf) + DPR + LOGIC_DPR + SHIFT(shift) + RM(rm) + OP2(op2)+ RN(rn) + RD(rd);
    if (strcmp(opc, "and") == 0)  { instr += OPC(andd); }
    if (strcmp(opc, "bic") == 0)  { instr += OPC(bic) + N; }
    if (strcmp(opc, "orr") == 0)  { instr += OPC(orr); }
    if (strcmp(opc, "orn") == 0)  { instr += OPC(orn) + N; }
    if (strcmp(opc, "eor") == 0)  { instr += OPC(eor); }
    if (strcmp(opc, "eon") == 0) { instr += OPC(eon) + N; }
    if (strcmp(opc, "ands") == 0)  { instr += OPC(ands); }
    if (strcmp(opc, "bics") == 0) { instr += OPC(bics) + N; }
    return instr;
}