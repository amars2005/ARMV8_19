//
// Created by sp3123 on 18/06/24.
//

#ifndef BITWISE_SHIFT_H
#define BITWISE_SHIFT_H

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {lsl, lsr, asr, ror} shiftType;

uint64_t lsl32(uint64_t rn, int shift_amount);
uint64_t lsr32(uint64_t rn, int shift_amount);
uint64_t asr32(uint64_t rn, int shift_amount);
uint64_t ror32(uint64_t rn, int shift_amount);
uint64_t lsl64(uint64_t rn, int shift_amount);
uint64_t lsr64(uint64_t rn, int shift_amount);
uint64_t asr64(uint64_t rn, int shift_amount);
uint64_t ror64(uint64_t rn, int shift_amount);
uint64_t bitwiseShift(uint64_t rn, int mode, shiftType instruction, int shift_amount);

#define LITTLE(i)  ((i & 0xFF) << 24) | \
                   ((i & 0xFF00) << 8) | \
                   ((i >> 8) & 0xFF00) | \
                   (i >> 24);

#endif