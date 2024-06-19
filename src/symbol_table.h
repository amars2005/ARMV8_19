#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <inttypes.h>

#define NEW_SYM_TABLE malloc(sizeof(symbolt))
#define LABEL_BUFFER 50

typedef struct symbolt_node *symbolt;

typedef struct {
    char label[LABEL_BUFFER];
    uint64_t   value;
} LVPair;

struct symbolt_node{
    LVPair* pair;
    symbolt next;
};

extern void addToTable(symbolt t, char* label, uint64_t value);
extern uint64_t find(symbolt t, char* label);
extern void firstPass(symbolt t, char** lines);

#endif