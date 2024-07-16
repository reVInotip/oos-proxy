CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

main.o: lib_stack.o loader.o
	$(CC) $(CCFLAGS) backend/main.c

include backend/shlib_operations/Makefile