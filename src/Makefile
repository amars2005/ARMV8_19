CC      ?= gcc
CFLAGS  ?= -g -std=c17\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic
LOADLIBES = -lm

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate

assemble: assemble.c tokenizer.c bitwise-shift.c sdthandler.c DPI-assembler.c branchingInstr.c symbol_table.c sdthandler.h
assemble: assemble.c tokenizer.c bitwise-shift.c sdthandler.c symbol_table.c DPI-assembler.c branchingInstr.c
	gcc $(LOADLIBES) $(CFLAGS) assemble.c symbol_table.c bitwise-shift.c tokenizer.c DPI-assembler.c branchingInstr.c sdthandler.c -o assemble
emulate: emulate.o
	gcc $(LOADLIBES) $(CFLAGS) emulate.c bitwise-shift.c -o emulate

clean:
	$(RM) *.o assemble emulate tokenizer bitwise-shift DPI-assembler branchingInstr symbol_table sdthandler
	
