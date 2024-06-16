#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <stdbool.h>

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

//void twoPass

// Converts each instruction to a uint32_t instruction stored in little Endian
uint32_t convertToLittleEndian(uint32_t instruction) {
  uint8_t bytes[4];
  uint32_t LE_instruction = 0;
  for (int i = 0; i < 4; i++) {
    bytes[i] = (uint8_t) ((instruction >> (i * 8)) & 0xFF);
    LE_instruction += bytes[i] << (24 - i * 8);
  }
  return LE_instruction;
}

// This writes all the instructions to a binary FILE
// Ensure that you have converted each instruction to little Endian
// before you parse it in to this function, all you have to do is
// call the convertToLittleEndian function
int writeToBinFile(char filename[], uint32_t instructions[]) {
  FILE *binFile = fopen(filename, "wb");
  if (binFile == NULL) {
    return 1;
  }

  int num_instructions = sizeof(instructions) / sizeof(uint32_t);
  int elementsWritten = fwrite(instructions, sizeof(uint32_t), num_instructions, binFile);
  if (elementsWritten != num_instructions) {
      fclose(binFile);
      return 2;
  }

  fclose(binFile);
  return 0;
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
    free(codeLines[i]);
  }
  free(codeLines);
  
  return EXIT_SUCCESS;
}
