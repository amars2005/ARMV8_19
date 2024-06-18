#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>

#include "instruction-types.h"

#define MAX_OPERANDS 10
#define MAX_LINE_LENGTH 100
#define MAX_OPERAND_LENGTH 10
#define MAX_OPCODE_LENGTH 10

/* 
splitLine is a data structure which represents a line which has been tokenised 
into opcode and operands. It also contains the address of the instruction because
we need it to build the splitLine into an instruction
*/
typedef struct {
    char *opcode;
    char (*operands)[MAX_OPERAND_LENGTH];
    int num_operands;
    int instruction_address;
} splitLine;

/*
tokenise_line takes a well-formed non-empty string and splits it into a splitLine data structure
*/
extern splitLine tokenize_line(char *line_in);

/*
line_to_instruction takes a splitLine structure and returns the corresponding instruction 
(the opcode determines the precise type of instruction and the operands are the same)
*/
extern instruction line_to_instruction(splitLine *data);

#endif