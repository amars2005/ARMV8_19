#include <stdint.h>
#include <stdbool.h>

// Structs representing different instruction types

typedef enum { arithmeticDPIt, wideMoveDPIt, arithmeticDPRt, logicDPRt, multiplyDPRt, brancht, bregt, bcondt, sdt, ll } instruction_t;
typedef enum { add, adds, sub, subs } arithmeticDPI_t;
typedef enum { and, orr, eor, ands} logicDPR_t;
typedef enum { bic, orn, eon, bics} logicDPRN_t;

typedef struct {
    bool      sf;
    uint64_t* Rd;
    uint64_t* Rn;
    uint64_t  Op2;
    uint64_t  opc;
} arithmeticDPI;

typedef struct {
    bool      sf;
    uint64_t  hw;
    uint64_t* Rd;
    uint64_t  Op;
    uint64_t opc;
} wideMoveDPI;

typedef struct {
    bool      sf;
    uint64_t* Rd;
    uint64_t* Rn;
    uint64_t Op2;
    uint64_t  opc;
} arithmeticDPR;

typedef struct {
    bool      sf;
    uint64_t* Rd;
    uint64_t* Rn;
    uint64_t Op2;
    uint64_t opc;
    bool N;
} logicDPR;

typedef struct {
    bool      sf;
    bool       X;
    uint64_t* Rd;
    uint64_t* Rn;
    uint64_t* Ra;
    uint64_t* Rm;
} multiplyDPR;

typedef struct {
    int64_t offset;
} branch;

typedef struct {
    uint64_t* Xn;
} breg;

typedef struct {
    int64_t offset;
    uint64_t cond;
} bcond;

typedef struct {
    bool sf;
    bool u;
    bool l;
    uint32_t offset;
    uint64_t* Xn;
    uint64_t* Rt;
} SDT;

typedef struct {
    bool sf;
    int32_t simm19;
    uint64_t* Rt;
} LL;

typedef union {
    arithmeticDPI arithmeticDpi;
    wideMoveDPI wideMoveDpi;
    arithmeticDPR arithmeticDpr;
    logicDPR logicDpr;
    multiplyDPR multiplyDpr;
    branch branch;
    breg breg;
    bcond bcond;
    SDT sdt;
    LL ll;
} instrData;

typedef struct {
    instrData     instruction;
    instruction_t itype;
} instruction;