#include <stdint.h>

extern uint32_t assembleUnCondBranch(int simm26);
extern uint32_t assembleRegisterBranch(int xn);
extern uint32_t assembleCondBranch(int simm19, char* cond);