#include <stdlib.h>


/*
call example: ./assemble <file_in> <file_out> i.e. ./assemble add01.s add01.bin

Me and Adam chose "Two Pass" implementation because its potentially simpler

PRE: assembly program being processed is syntactically correct and all labels 
     and instruction mnemonics are written lower case

Task breakdown: 
• Constructing a binary file writer.
• Building a symbol table abstract data type (ADT).
• Designing and constructing the assembler, comprising:
– An assembly (.s) file reader, that is able to send each line to the parser
- An instruction parser, that is able to convert a string line into an instruction.
– A tokenizer for breaking a line into its label, opcode and operand field(s) (you might find the
strtok_r and strtol functions helpful here).
– An instruction assembler. for example:
* A function for assembling Data Processing instructions.
* A function for assembling Load and Store instructions.
* A function for assembling Branch instructions.
• An implementation of the one or two-pass assembly process
*/
int main(int argc, char **argv) {
  return EXIT_SUCCESS;
}
