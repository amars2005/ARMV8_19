#ifndef TOKENIZER_H
#define TOKENIZER_H

#define MAX_OPERANDS 10
#define MAX_LINE_LENGTH 100
#define MAX_OPERAND_LENGTH 10
#define MAX_OPCODE_LENGTH 10

typedef struct {
    char *opcode;
    char (*operands)[MAX_OPERAND_LENGTH];
    int numOperands;
} splitInstr;

extern splitInstr tokenize_line(char *line_in);

#endif