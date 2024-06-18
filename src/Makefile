CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic
LOADLIBES = -lm

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate

assemble: assemble.c tokenizer.c bitwise-shift.c
	gcc $(CFLAGS) ./assemble.c ./bitwise-shift.c ./tokenizer.c -o assemble
emulate: emulate.o

clean:
	$(RM) *.o assemble emulate tokenizer
	
