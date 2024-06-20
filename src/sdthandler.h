#include "instruction-types.h"

#define LITTLE(i)  ((i & 0xFF) << 24) | \
                   ((i & 0xFF00) << 8) | \
                   ((i >> 8) & 0xFF00) | \
                   (i >> 24);

extern instruction SDTbuilder(char *type, uint8_t rt, char *address, uint8_t sf);
extern uint32_t assembleLL(LL literal);
extern uint32_t assembleIndexSDT(SDTindex index);
extern uint32_t assembleUOffsetSDT(SDTuOffset uoffset);
extern uint32_t assembleRegOffsetSDT(SDTregOffset regoffset);