#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "symbol_table.h"
#include "branchingInstr.h"
#include "tokenizer.h"
#include "instruction-types.h"
#include "DPI-assembler.h"
#include "sdthandler.h"

// This is the size of buffer for loading in chars from input file
// We will double the size if a line is greater than 64 chars
#define INITIAL_BUFFER_SIZE 64
// This is buffer size for number of lines in program
// Double this number if program is greater than 16 lines
#define INITIAL_NUM_LINES 16

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

// This function reads in a file and returns list of strings
// Each element in the list corresponds to each line 
char **readFile(FILE *file) {
  // Create a buffer to temporarily store each line
  char *buffer;
  buffer = (char *)malloc(sizeof(char) * INITIAL_BUFFER_SIZE);
  if (buffer == NULL) {
    fclose(file);
    return NULL;
  }
  int buffer_size = INITIAL_BUFFER_SIZE;

  // Create empty list of strings to store each line of the .s file
  char **lines = (char **)malloc(INITIAL_NUM_LINES * sizeof(char *));
  if (lines == NULL) {
    fclose(file);
    return NULL;
  }
  int line_buffer_size = INITIAL_NUM_LINES;
  int c;
  int index = 0;
  int index_lines = 0;
  
  // Iterate through each character of file
  while ((c = fgetc(file)) != EOF) {
    // Check if buffer needs to be resized (this occurs if line being read is too long)
    if (index >= buffer_size - 1) {
      buffer_size *= 2;
      buffer = realloc(buffer, buffer_size * sizeof(char));
      if (buffer == NULL) {
        fclose(file);
        return NULL;
      }
    }
    // This checks if c is at end of line
    if (c == '\n') {
      // Add null terminator to establish end of string
      buffer[index] = '\0';
      index = 0;
      // Check if lines array must be resized (occurs if number of lines in program is large)
      if (index_lines >= line_buffer_size - 1) {
        line_buffer_size *= 2;
        lines = realloc(lines, line_buffer_size * sizeof(char *));
        if (lines == NULL) {
          fclose(file);
          return NULL;
        }
      }
      // Ensure a copy of the buffer is created which is stored in lines array
      int length = strlen(buffer);
      char *copy = (char *)malloc((length + 1) * sizeof(char));
      if (copy == NULL) {
        fclose(file);
        return NULL;
      }
      strcpy(copy, buffer);
      lines[index_lines] = copy;
      index_lines++;
      // Free buffer so it can be reallocated for the next line being read in
      free(buffer);
      buffer = (char *)malloc(sizeof(char) * buffer_size);
      if (buffer == NULL) {
        fclose(file);
        return NULL;
      }

    } else {
      // Copy each character of each line into the buffer
      buffer[index] = (char) c;
      index++;
    }

  }

  // c = EOF so ensure last line has a '\0' appended to it
  // and is added to the list
  buffer[index] = '\0';

  if (index_lines > line_buffer_size - 2) {
    line_buffer_size *= 2;
    lines = realloc(lines, line_buffer_size);
    if (lines == NULL) {
      fclose(file);
      return NULL;
    }
  }
  int length = strlen(buffer);
  char *copy = (char *)malloc(buffer_size * sizeof(char));
  if (copy == NULL) {
    fclose(file);
    return NULL;
  }
  strcpy(copy, buffer);
  copy[length] = '\0';
  lines[index_lines] = copy;
  free(buffer);
  lines[++index_lines] = NULL;
  return lines;
}

void secondPass(char** lines, uint32_t* instrs, symbolt symbol_table) {
    for (int j = 0; lines[j] != NULL; j ++) {
        splitLine l_splitLine = tokenize_line(lines[j], j*4);
        instruction i = line_to_instruction(&l_splitLine, symbol_table);
        switch (i.itype) {
            case arithmeticDPIt:
                instrs[j] = assembleArithmeticDPI(i.instruction.arithmeticDpi.opc, *i.instruction.arithmeticDpi.Rd, *i.instruction.arithmeticDpi.Rn, i.instruction.arithmeticDpi.Op2, i.instruction.arithmeticDpi.sf);
                break;
            case wideMoveDPIt:
                instrs[j] = assembleWideMoveDPI(i.instruction.wideMoveDpi.opc, *i.instruction.wideMoveDpi.Rd, i.instruction.wideMoveDpi.Op, i.instruction.arithmeticDpi.sf);
                break;
            case arithmeticDPRt:
                instrs[j] = assembleArithmeticDPR(i.instruction.arithmeticDpr.opc, *i.instruction.arithmeticDpr.Rd, *i.instruction.arithmeticDpr.Rn, *i.instruction.arithmeticDpr.Rm, i.instruction.arithmeticDpr.Shift, i.instruction.arithmeticDpi.sf);
                break;
            case logicDPRt:
                instrs[j] = assembleLogicDPR(i.instruction.logicDpr.opc, *i.instruction.logicDpr.Rd, *i.instruction.logicDpr.Rn, *i.instruction.logicDpr.Rm, i.instruction.logicDpr.Shift, i.instruction.logicDpr.N, i.instruction.logicDpr.sf);
                break;
            case multiplyDPRt:
                instrs[j] = assembleMultiply(i.instruction.multiplyDpr.X, *i.instruction.multiplyDpr.Rd, *i.instruction.multiplyDpr.Rn, *i.instruction.multiplyDpr.Rm, *i.instruction.multiplyDpr.Ra, i.instruction.multiplyDpr.sf);
                break;
            case brancht:
                instrs[j] = assembleUnCondBranch(i.instruction.branch.offset);
                break;
            case bregt:
                instrs[j] = assembleRegisterBranch(*i.instruction.breg.Xn);
                break;
            case bcondt:
                instrs[j] = assembleCondBranch(i.instruction.bcond.offset, i.instruction.bcond.cond);
                break;
            case sdtIndex:
                instrs[j] = assembleIndexSDT(i.instruction.sdtindex);
                break;
            case sdtRegOffset:
                instrs[j] = assembleRegOffsetSDT(i.instruction.sdtregoffset);
                break;
            case sdtUOffset:
                instrs[j] = assembleUOffsetSDT(i.instruction.sdtuoffset);
                break;
            case ll:
                instrs[j] = assembleLL(i.instruction.ll);
                break;
            case directive:
                instrs[j] = i.instruction.directive;
                break;
        }
    }
}

int main(int argc, char **argv) {
  // validate input arguments
  if (argc != 3) { 
    fprintf(stderr, "Usage: ./assemble <file_in> <file_out>\n");
    exit(1);
  }

  // read in the assembly code so it can create symbol table
  FILE *assemblyFile = fopen(argv[1], "r");
  assert(assemblyFile != NULL);

  char **codeLines = readFile(assemblyFile);

  if (codeLines != NULL) {
    fclose(assemblyFile);
  } else {
    return EXIT_FAILURE;
  }
  
  // Calculate size of codeLines and free memory at end
  int size = -1;
  while(codeLines[++size] != NULL) {}
  for (int i = 0; i < size; i++) {
    printf("%s\n", codeLines[i]);
    free(codeLines[i]);
  }
  free(codeLines);

  return EXIT_SUCCESS;
}