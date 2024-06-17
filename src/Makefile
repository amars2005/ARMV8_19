CC      ?= gcc
CFLAGS  ?= -g -std=c17\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic
LOADLIBES = -lm

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate

assemble: assemble.c symbol_table.c branchingInstr.c
	gcc $(CFLAGS) assemble.c symbol_table.c branchingInstr.c -o assemble
emulate: emulate.o

clean:
	$(RM) *.o assemble emulate
	
