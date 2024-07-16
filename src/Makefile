CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

LDFLAGS = -ldl

all: main.o lib_stack.o loader.o
	$(CC) $(LDFLAGS) lib_stack.o loader.o main.o -o oos_proxy

include backend/Makefile

clean:
	rm -rf *.o