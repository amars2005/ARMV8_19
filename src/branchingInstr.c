#include "branchingInstr.h"
#include <string.h>
#include <stdint.h>

uint32_t unCondBranch(int simm26) {
    uint32_t return_val = ((5<<26) + simm26);
    return return_val;
}

uint32_t registerBranch(int xn) {
    uint32_t return_val = (uint32_t) ((54815<<16)+(xn<<5));
    return return_val;
}

uint32_t condBranch(int simm19, char* cond) {
    uint32_t return_val = (uint32_t) (21<<26) + (uint32_t) (simm19<<5);
    if (strcmp(cond, "EQ")) { return return_val; }
    else if (strcmp(cond, "NE")) { return return_val + 1; } 
    else if (strcmp(cond, "GE")) { return return_val + 10; }
    else if (strcmp(cond, "LT")) { return return_val + 11; }
    else if (strcmp(cond, "GT")) { return return_val + 12; }
    else if (strcmp(cond, "LE")) { return return_val + 13; }
    else if (strcmp(cond, "AL")) { return return_val + 14; }
    else { return return_val; }
}
