#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <inttypes.h>
#include <string.h>
#include "symbol_table.h"


typedef struct {
    char label[LABEL_BUFFER];
    uint64_t   value;
} LVPair;

struct symbolt_node{
    LVPair* pair;
    symbolt next;
};

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
    t = t->next;
    while (t != NULL) {
        if (strcmp(label, t->pair->label) == 0) {
            return t->pair->value;
        }
        t = t->next;
    }
    fprintf(stderr, "Label: %s not found", label);
    exit(1);
}

void fistPass(symbolt t, char** lines) {
    char* label_regex_str = "[a-zA-Z_.][a-zA-Z0-9$_.]*:";
    regex_t label_regex;
    int value = regcomp(&label_regex, label_regex_str, 0);

    if (value != 0) {
        fprintf(stderr, "Regex compilation failed\n");
        exit(1);
    }

    int instr_num = 0;
    for (char** line = lines; *line != NULL; line ++) {
        if (regexec(&label_regex, *line, 0, NULL, 0) == REG_NOMATCH) {
            instr_num ++;
            continue;
        }

        char label[strlen(*line)];
        strcpy(label, *line);
        label[strlen(label)-1] = '\0';
        int addr = (instr_num + 1) * 4;

        addToTable(t, label, addr);
    }
}