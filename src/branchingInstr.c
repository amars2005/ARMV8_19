#include "branchingInstr.h"
#include <stdint.h>

uint32_t assembleUnCondBranch(int simm26) {
    simm26 &= MASK26;
    uint32_t return_val = ((5<<26) + simm26);
    return LITTLE(return_val);
}

uint32_t assembleRegisterBranch(uint64_t xn) {
    uint32_t return_val = ((54815<<16)+(xn<<5));
    return LITTLE(return_val);
}

uint32_t assembleCondBranch(int64_t simm19, int cond) {
    simm19 &= MASK19;
    uint32_t return_val = (uint32_t) (21<<26) + (uint32_t) (simm19<<5) + cond;
    return LITTLE(return_val);
}
