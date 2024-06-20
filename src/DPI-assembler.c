#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "instruction-types.h"
#include "DPI-assembler.h"

uint32_t assembleMultiply(bool x, uint64_t rd, uint64_t rn, uint64_t rm, uint64_t ra, bool sf) {
    uint32_t instr = SF(sf) + MUL + RM(rm) + RA(ra) + RN(rn) + RD(rd);
    if (x) { instr += X_BIT; }
    return instr;
}

uint32_t assembleArithmeticDPI(arithmeticDPI_t opc, uint64_t rd, uint64_t rn, uint64_t imm12, bool sh, bool sf) {
    uint32_t instr = SF(sf) + DPI + ARITHM_DPI + OPC(opc) + SH(sh) + IMM12(imm12) + RN(rn) + RD(rd);
    return instr;
}

uint32_t assembleWideMoveDPI(uint64_t opc, uint64_t rd, uint64_t imm16, uint64_t hw, bool sf) {
    uint32_t x1 = SF(sf);
    uint32_t x2 = DPI;
    uint32_t x3 = WMOVE_DPI;
    uint32_t x4 = OPC(opc);
    uint32_t x5 = HW(hw);
    uint32_t x6 = IMM16(imm16);
    uint32_t x7 = RD(rd);
    uint32_t instr = x1 + x2 + x3 + x4 + x5 + x6 + x7;
    return instr;
}

uint32_t assembleArithmeticDPR(arithmeticDPI_t opc, uint64_t rd, uint64_t rn, uint64_t rm, uint64_t shift, bool sf) {
    uint32_t instr = SF(sf) + DPR + ARITHM_DPR + OPC(opc) + RM(rm) + SHIFT(shift) + RN(rn) + RD(rd);
    return instr;
}

uint32_t assembleLogicDPR(uint64_t opc, uint64_t rd, uint64_t rn, uint64_t rm, uint64_t shift, uint64_t operand, bool n, bool sf) {
    uint32_t instr = SF(sf) + DPR + LOGIC_DPR + OPC(opc) + OPERAND(operand) + RM(rm) + SHIFT(shift) + RN(rn) + RD(rd);
    if (n) { instr += N_BIT; }
    return instr;
}