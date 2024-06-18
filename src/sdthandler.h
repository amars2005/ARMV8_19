#include "instruction-types.h"

extern instruction SDTbuilder(char *type, uint8_t rt, char *address, uint8_t sf);
extern uint32_t LLBinary(LL literal);
extern uint32_t indexBinary(SDTindex index);
extern uint32_t uOffsetBinary(SDTuOffset uoffset);
extern uint32_t regOffsetBinary(SDTregOffset regoffset);