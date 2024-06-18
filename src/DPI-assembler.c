#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "DPI-assembler.h"

uint32_t assembleMultiply(bool x, uint64_t rd, uint64_t rn, uint64_t rm, uint64_t ra, bool sf) {
    uint32_t instr = SF(sf) + MUL + RM(rm) + RA(ra) + RN(rn) + RD(rd);
    if (x) { instr += X_BIT; }
    return instr;
}

uint32_t assembleArithmeticDPI(arithmeticDPI_t opc, uint64_t rd, uint64_t rn, uint64_t imm12, bool sf) {
    bool sh = false;
    if (imm12 > IMM12_SIZE) {
        imm12 >>= 12;
        sh = true;
    }
    uint32_t instr = SF(sf) + DPI + ARITHM_DPI + OPC(opc) + SH(sh) + IMM12(imm12) + RN(rn) + RD(rd);
    return instr;
}

uint32_t assembleWideMoveDPI(uint64_t opc, uint64_t rd, uint64_t imm16, bool sf) {
    int hw = 0;
    int mask = (1 << 16) - 1;
    while ((imm16 & mask) == 0) {
        hw ++;
        imm16 >>= 16;
    }
    assert(hw < 4);
    uint32_t instr = SF(sf) + DPI + WMOVE_DPI + OPC(opc) + HW(hw) + IMM16(imm16) + RD(rd);
    return instr;
}

uint32_t assembleArithmeticDPR(arithmeticDPI_t opc, uint64_t rd, uint64_t rn, uint64_t rm, uint64_t shift, bool sf) {
    uint32_t instr = SF(sf) + DPR + ARITHM_DPR + OPC(opc) + RM(rm) + SHIFT(shift) + RN(rn) + RD(rd);
    return instr;
}

uint32_t assembleLogicDPR(uint64_t opc, uint64_t rd, uint64_t rn, uint64_t rm, uint64_t shift, bool n, bool sf) {
    uint32_t instr = SF(sf) + DPR + LOGIC_DPR + OPC(opc) + RM(rm) + SHIFT(shift) + RN(rn) + RD(rd);
    if (n) { instr += N_BIT; }
    return instr;
}