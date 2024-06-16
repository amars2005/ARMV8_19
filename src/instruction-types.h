#include <stdint.h>
#include <stdbool.h>

// Structs representing different instruction types
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