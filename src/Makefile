CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic
LOADLIBES = -lm

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate tokenizer

assemble: assemble.o
emulate: emulate.o
tokenizer: tokenizer.o

clean:
	$(RM) *.o assemble emulate tokenizer
	
