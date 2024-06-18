//
// Created by sp3123 on 18/06/24.
//
// Functions relating to bitwise shifts (1.6)

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "bitwise-shift.h"

uint64_t lsl32(uint64_t rn, int shift_amount) {
    uint32_t lowerBits = (uint32_t) (rn & 0xFFFFFFFF);
    return (uint64_t) (lowerBits << shift_amount);
}

uint64_t lsr32(uint64_t rn, int shift_amount) {
    uint32_t lowerBits = (uint32_t) (rn & 0xFFFFFFFF);
    return (uint64_t) (lowerBits >> shift_amount);
}

uint64_t asr32(uint64_t rn, int shift_amount) {
    int32_t lowerBits = (int32_t) (rn & 0xFFFFFFFF);
    return (uint64_t) (lowerBits >> shift_amount);
}

uint64_t ror32(uint64_t rn, int shift_amount) {
    int32_t lowerBits = (int32_t) (rn & 0xFFFFFFFF);
    for (int i = 0; i < shift_amount; i++) {
        uint32_t lsb = lowerBits & 0x00000001;
        lowerBits >>= 1;
        if (lsb != 0) {
            lowerBits |= 0x80000000;
        } else {
            lowerBits &= 0x7FFFFFFF;
        }
    }
    return lowerBits;
}

uint64_t lsl64(uint64_t rn, int shift_amount) {
    return (rn << shift_amount);
}

uint64_t lsr64(uint64_t rn, int shift_amount) {
    return (rn >> shift_amount);
}

uint64_t asr64(uint64_t rn, int shift_amount) {
    int64_t rnSigned = (int64_t) rn;
    return (rnSigned >> shift_amount);
}

uint64_t ror64(uint64_t rn, int shift_amount) {
    for (int i = 0; i < shift_amount; i++) {
        uint64_t lsb = rn & 0x0000000000000001;
        rn >>= 1;
        if (lsb != 0) {
            rn |= 0x8000000000000000;
        } else {
            rn &= 0x7FFFFFFFFFFFFFFF;
        }
    }
    return rn;
}

// First input is the register number
// Second input is the register mode (32 or 64 bit) represented by 0 and 1 respectively
// Third input is the instruction type (lsl, lsr, asr, ror)
// 0, 1, 2, 3 for lsl, lsr, asr, ror respectively

uint64_t bitwiseShift(uint64_t rn, int mode, shiftType instruction, int shift_amount) {
    assert(instruction >= 0 && instruction <= 3);

    assert(mode == 0 || mode == 1);

    if (mode == 0) {
        switch (instruction) {
            case lsl: return lsl32(rn, shift_amount);
            case lsr: return lsr32(rn, shift_amount);
            case asr: return asr32(rn, shift_amount);
            case ror: return ror32(rn, shift_amount);
            default:
                fprintf(stderr, "Unknown shift\n");
                exit(1);
        }
    } else {
        switch (instruction) {
            case lsl: return lsl64(rn, shift_amount);
            case lsr: return lsr64(rn, shift_amount);
            case asr: return asr64(rn, shift_amount);
            case ror: return ror64(rn, shift_amount);
            default:
                fprintf(stderr, "Unknown shift\n");
                exit(1);
        }
    }
}