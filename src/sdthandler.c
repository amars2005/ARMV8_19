#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include "instruction-types.h"


instruction loadLiteralBuilder(uint8_t rt, char *address, uint8_t sf) {
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

instruction preIndexBuilder(char *type, uint8_t rt, char *address, uint8_t sf) {
    instruction inst;
    inst.itype = sdtIndex;
    instrData data;
    SDTindex preIndex;

    int hashtagIndex = 0;
    while (address[0] != '#') {
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

instruction postIndexBuilder(char *type, uint8_t rt, char *address, uint8_t sf) {
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
    while (address[0] != '#') {
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

instruction unsignedOffsetBuilder(char *type, uint8_t rt, char *address, uint8_t sf) {
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
    while (address[0] != '#') {
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

    uoffset.imm12 = imm12;

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

instruction registerOffsetBuilder(char *type, uint8_t rt, char *address, uint8_t sf) {
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
    while (address[0] != '#') {
        commaIndex++;
    }
    uint64_t xm;
    if (address[commaIndex + 4] == ']') {
        char n[1] = {address[commaIndex + 3]};
        xn = atoi(n);
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
    if (address[0] == '#') {
        return loadLiteralBuilder(rt, address, sf);
    } else if (address[strlen(address) - 1] == '!') {
        return preIndexBuilder(type, rt, address, sf);
    } else if (address[strlen(address) - 2] == '}') {
        return unsignedOffsetBuilder(type, rt, address, sf);
    } else if (address[strlen(address) - 1] == ']') {
        return registerOffsetBuilder(type, rt, address, sf);
    } else {
        return postIndexBuilder(type, rt, address, sf);
    }
}