CC      ?= gcc
CFLAGS  ?= -g -std=c17\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic
LOADLIBES = -lm

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate symbol_table

assemble: assemble.o
emulate: emulate.o
symbol_table: symbol_table.o

clean:
	$(RM) *.o assemble emulate symbol_table
	
