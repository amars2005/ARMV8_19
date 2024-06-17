#include <stdint.h>

extern uint32_t unCondBranch(int simm26);
extern uint32_t registerBranch(int xn);
extern uint32_t condBranch(int simm19, char* cond);