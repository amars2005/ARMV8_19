#include <stdint.h>

#define EQ 0 // 0000
#define NE 1 // 0001
#define GE 10 // 1010
#define LT 11 // 1011
#define GT 12 // 1100
#define LE 13 // 1101
#define AL 14 // 1110

extern uint32_t assembleUnCondBranch(int simm26);
extern uint32_t assembleRegisterBranch(uint64_t xn);
extern uint32_t assembleCondBranch(int64_t simm19, int cond);