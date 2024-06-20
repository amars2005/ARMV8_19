#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include "instruction-types.h"
#include "sdthandler.h"


static instruction loadLiteralBuilder(uint8_t rt, char *address, uint8_t sf) {
    instruction inst;
    inst.itype = ll;
    instrData data;
    LL literal;
    if (sf == 0) {
        literal.sf = false;
    } else {
        literal.sf = true;
    }
    char *simm19temp = address + 1;
    uint32_t simm19 = atoi(simm19temp);
    literal.simm19 = simm19;
    literal.Rt = (uint64_t) rt;
    data.ll = literal;
    inst.instruction = data;

    return inst;
}

static instruction zeroOffsetBuilder(char *type, uint8_t rt, char *address, uint8_t sf) {
    instruction inst;
    inst.itype = sdtUOffset;
    instrData data;
    SDTuOffset uoffset;

    uint64_t xn;
    if (address[3] == ']') {
        char n[1] = {address[2]};
        xn = atoi(n);
    } else {
        char n[2] = {address[2], address[3]};
        xn = atoi(n);
    }
    uoffset.Xn = xn;
    uoffset.imm12 = 0;

    if (strcmp(type, "ldr") == 0) {
        uoffset.l = 1;
    } else {
        uoffset.l = 0;
    }

    if (sf == 0) {
        uoffset.sf = false;
    } else {
        uoffset.sf = true;
    }

    uoffset.u = true;
    uoffset.Rt = (uint64_t) rt;
    data.sdtuoffset = uoffset;
    inst.instruction = data;

    return inst;
}

static instruction preIndexBuilder(char *type, uint8_t rt, char *address, uint8_t sf) {
    instruction inst;
    inst.itype = sdtIndex;
    instrData data;
    SDTindex preIndex;

    int hashtagIndex = 0;
    while (address[hashtagIndex] != '#') {
        hashtagIndex++;
    }
    char *simm9temp = (char *)malloc(strlen(address) - hashtagIndex);
    int index = hashtagIndex + 1;
    while (isdigit(address[index])) {
        simm9temp[index - hashtagIndex - 1] = address[index];
        index++;
    }
    simm9temp[index - hashtagIndex] = '\0';
    uint32_t simm9 = atoi(simm9temp);
    free(simm9temp);
    
    preIndex.simm9 = simm9;
    preIndex.i = 1;
    preIndex.u = 0;

    if (strcmp(type, "ldr") == 0) {
        preIndex.l = 1;
    } else {
        preIndex.l = 0;
    }

    preIndex.Rt = rt;

    uint64_t xn;
    if (address[3] == ',') {
        char n[1] = {address[2]};
        xn = atoi(n);
    } else {
        char n[2] = {address[2], address[3]};
        xn = atoi(n);
    }
    preIndex.Xn = xn;

    if (sf == 0) {
        preIndex.sf = false;
    } else {
        preIndex.sf = true;
    }

    data.sdtindex = preIndex;
    inst.instruction = data;

    return inst;
}

static instruction postIndexBuilder(char *type, uint8_t rt, char *address, uint8_t sf) {
    instruction inst;
    inst.itype = sdtIndex;
    instrData data;
    SDTindex postindex;

    uint64_t xn;
    if (address[3] == ']') {
        char n[1] = {address[2]};
        xn = atoi(n);
    } else {
        char n[2] = {address[2], address[3]};
        xn = atoi(n);
    }
    postindex.Xn = xn;

    int hashtagIndex = 0;
    while (address[hashtagIndex] != '#') {
        hashtagIndex++;
    }
    char *simm9temp = (char *)malloc(strlen(address) - hashtagIndex);
    int index = hashtagIndex + 1;
    while (isdigit(address[index])) {
        simm9temp[index - hashtagIndex - 1] = address[index];
        index++;
    }
    simm9temp[index - hashtagIndex] = '\0';
    uint32_t simm9 = atoi(simm9temp);
    free(simm9temp);

    postindex.simm9 = simm9;
    postindex.i = 1;
    postindex.u = 0;

    if (strcmp(type, "ldr") == 0) {
        postindex.l = 1;
    } else {
        postindex.l = 0;
    }

    postindex.Rt = rt;

    if (sf == 0) {
        postindex.sf = false;
    } else {
        postindex.sf = true;
    }

    data.sdtindex = postindex;
    inst.instruction = data;

    return inst;
} 

static instruction unsignedOffsetBuilder(char *type, uint8_t rt, char *address, uint8_t sf) {
    instruction inst;
    inst.itype = sdtUOffset;
    instrData data;
    SDTuOffset uoffset;

    uint64_t xn;
    if (address[3] == '{') {
        char n[1] = {address[2]};
        xn = atoi(n);
    } else {
        char n[2] = {address[2], address[3]};
        xn = atoi(n);
    }
    uoffset.Xn = xn;

    int hashtagIndex = 0;
    while (address[hashtagIndex] != '#') {
        hashtagIndex++;
    }
    char *imm12temp = (char *)malloc(strlen(address) - hashtagIndex);
    int index = hashtagIndex + 1;
    while (isdigit(address[index])) {
        imm12temp[index - hashtagIndex - 1] = address[index];
        index++;
    }
    imm12temp[index - hashtagIndex] = '\0';
    uint32_t imm12 = atoi(imm12temp);
    free(imm12temp);

    uoffset.imm12 = imm12 >> (2 + sf);

    if (strcmp(type, "ldr") == 0) {
        uoffset.l = 1;
    } else {
        uoffset.l = 0;
    }

    if (sf == 0) {
        uoffset.sf = false;
    } else {
        uoffset.sf = true;
    }

    uoffset.u = true;
    uoffset.Rt = (uint64_t) rt;
    data.sdtuoffset = uoffset;
    inst.instruction = data;

    return inst;
}

static instruction registerOffsetBuilder(char *type, uint8_t rt, char *address, uint8_t sf) {
    instruction inst;
    inst.itype = sdtRegOffset;
    instrData data;
    SDTregOffset regoffset;

    uint64_t xn;
    if (address[3] == ',') {
        char n[1] = {address[2]};
        xn = atoi(n);
    } else {
        char n[2] = {address[2], address[3]};
        xn = atoi(n);
    }
    regoffset.Xn = xn;

    int commaIndex = 0;
    while (address[commaIndex] != '#') {
        commaIndex++;
    }
    uint64_t xm;
    if (address[commaIndex + 4] == ']') {
        char n[1] = {address[commaIndex + 3]};
        xm = atoi(n);
    } else {
        char n[2] = {address[commaIndex + 3], address[commaIndex + 4]};
        xm = atoi(n);
    }
    regoffset.Xm = xm;

    if (strcmp(type, "ldr") == 0) {
        regoffset.l = 1;
    } else {
        regoffset.l = 0;
    }

    if (sf == 0) {
        regoffset.sf = false;
    } else {
        regoffset.sf = true;
    }

    regoffset.u = false;
    regoffset.Rt = (uint64_t) rt;
    data.sdtregoffset = regoffset;
    inst.instruction = data;

    return inst;
}

instruction SDTbuilder(char *type, uint8_t rt, char *address, uint8_t sf) {
    // Need to decode the type of address
    char hash = '#';
    int len = strlen(address);
    if (address[0] == '#') {
        return loadLiteralBuilder(rt, address, sf);
    } else if (address[len - 1] == '!') {
        return preIndexBuilder(type, rt, address, sf);
    } else if (address[len - 1] == ']' && strchr(address, hash) != NULL) {
        return unsignedOffsetBuilder(type, rt, address, sf);
    } else if (address[strlen(address) - 1] == ']') {
        return registerOffsetBuilder(type, rt, address, sf);
    } else if (address[len - 1] == ']' && len < 6) {
        return zeroOffsetBuilder(type, rt, address, sf);
    } else {
        return postIndexBuilder(type, rt, address, sf);
    }
}

uint32_t assembleLL(LL literal) {
    uint32_t instruction = 0;
    instruction += (uint32_t) literal.Rt;
    instruction += literal.simm19 << 5;
    instruction += 3 << 27;
    if (literal.sf == true) {
        instruction += 1 << 30;
    }
    return instruction;
}

uint32_t assembleIndexSDT(SDTindex index) {
    uint32_t instruction = 0;
    instruction += (uint32_t) index.Rt;
    instruction += (uint32_t) index.Xn << 5;
    if (index.i == true) {
        instruction += 3 << 10;
    } else {
        instruction += 1 << 10;
    }
    instruction += index.simm9 << 12;
    if (index.l == true) {
        instruction += 1 << 22;
    }
    instruction += 7 << 27;
    if (index.sf == true) {
        instruction += 3 << 30;
    } else {
        instruction += 2 << 30;
    }

    return instruction;
}

uint32_t assembleUOffsetSDT(SDTuOffset uoffset) {
    uint32_t instruction = 0;
    instruction += (uint32_t) uoffset.Rt;
    instruction += (uint32_t) uoffset.Xn << 5;
    instruction += uoffset.imm12 << 10;
    if (uoffset.l == true) {
        instruction += 1 << 22;
    }
    instruction += 1 << 24;
    instruction += 7 << 27;
    if (uoffset.sf == true) {
        instruction += 3 << 30;
    } else {
        instruction += 2 << 30;
    }

    return instruction;
}

uint32_t assembleRegOffsetSDT(SDTregOffset regoffset) {
    uint32_t instruction = 0;
    instruction += (uint32_t) regoffset.Rt;
    instruction += (uint32_t) regoffset.Xn << 5;
    instruction += 13 << 11;
    instruction += (uint32_t) regoffset.Xm << 16;
    if (regoffset.l == true) {
        instruction += 3 << 21;
    } else {
        instruction += 1 << 21;
    }
    instruction += 7 << 27;
    if (regoffset.sf == true) {
        instruction += 3 << 30;
    } else {
        instruction += 2 << 30;
    }

    return instruction;
}