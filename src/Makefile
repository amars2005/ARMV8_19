CC      ?= gcc
CFLAGS  ?= -g -std=c17\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic
LOADLIBES = -lm

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate

assemble: assemble.c tokenizer.c bitwise-shift.c
	gcc $(CFLAGS) assemble.c bitwise-shift.c tokenizer.c DPI-assemble.c branchingInstr.c symbol_table.c -o assemble
emulate: emulate.o
	gcc $(CFLAGS) emulate.c bitwise-shift.c -o emulate

clean:
	$(RM) *.o assemble emulate tokenizer bitwise-shift DPI-assemble branchingInstr symbol_table
	
