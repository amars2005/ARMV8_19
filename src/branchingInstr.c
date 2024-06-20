#include "branchingInstr.h"
#include <string.h>
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
    uint32_t return_val = (uint32_t) (21<<26) + (uint32_t) (simm19<<5);
    switch (cond) {
        case EQ:
            return LITTLE(return_val);
        case NE:
            return LITTLE((return_val + 1));
        case GE:
            return LITTLE((return_val + 10));
        case LT:
            return LITTLE((return_val + 11));
        case GT:
            return LITTLE((return_val + 12));
        case LE:
            return LITTLE((return_val + 13));
        case AL:
            return LITTLE((return_val + 14));
        default:
            return LITTLE(return_val);
    }
}
