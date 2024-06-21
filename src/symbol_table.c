#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <inttypes.h>
#include <string.h>
#include "tokenizer.h"
#include "symbol_table.h"

void addToTable(symbolt t, char* label, uint64_t value) {
    LVPair* pair = malloc(sizeof(LVPair));
    pair->value = value;
    strcpy(pair->label, label);
    while (t->next != NULL) {
        t = t->next;
    }
    t->next = malloc(sizeof(symbolt));
    t->next->pair = pair;
    t->next->next = NULL;
}

uint64_t find(symbolt t, char* label) {
    while (t != NULL) {
        if (!strcmp(label, t->pair->label)) {
            return t->pair->value;
        }
        t = t->next;
    }
    return -1;
}

void freeTable(symbolt t) {
    if (t->next != NULL){ freeTable(t->next); }
    free(t);
}

void firstPass(symbolt t, char** lines) {
    int instr_num = 0;
    for (char** line = lines; *line != NULL; line ++) {
        if (!isLabelColon(*line)) {
            instr_num ++;
            continue;
        }

        char label[strlen(*line) + 1];
        strcpy(label, *line);
        
        int i = strlen(label);
        while (label[i] != ':') { i--; }
        label[i] = '\0';
        int addr = instr_num;

        addToTable(t, label, addr);
    }
}