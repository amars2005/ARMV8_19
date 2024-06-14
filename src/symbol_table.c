#include <stdlib.h>
#include <stdio.h>
#include "symbol_table.h"

#define NEW_SYM_TABLE malloc(sizeof(symbolt))

typedef struct {
    char* label;
    int   value;
} LVPair;

struct symbolt{
    LVPair pair;
    struct symbolt* next;
};

typedef struct symbolt *symbolt;

void addToTable(symbolt t, char* label, int value) {
    LVPair pair = { .label = label, .value = value };
    while (t->next != NULL) {
        t = t->next;
    }
    t->next = malloc(sizeof(symbolt));
    t->next->pair = pair;
    t->next->next = NULL;
}

int find(symbolt t, char* label) {
    while (t != NULL) {
        if (label == t->pair.label) {
            return t->pair.value;
        }
        t = t->next;
    }
    fprintf(stderr, "Label: %s not found", label);
    exit(1);
}