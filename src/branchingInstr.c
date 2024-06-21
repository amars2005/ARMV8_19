#include "branchingInstr.h"
#include <stdint.h>

uint32_t assembleUnCondBranch(int simm26) {
    uint32_t return_val = LITTLE(((5<<26) + simm26));
    return return_val;
}

uint32_t assembleRegisterBranch(uint64_t xn) {
    uint32_t return_val = LITTLE(((54815<<16)+(xn<<5)));
    return return_val;
}

uint32_t assembleCondBranch(int64_t simm19, int cond) {
    uint32_t return_val = (uint32_t) (21<<26) + (uint32_t) (simm19<<5) + cond;
    return LITTLE(return_val);
}
