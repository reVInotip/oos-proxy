CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

loader.o: lib_stack.o
	$(CC) $(CCFLAGS) backend/shlib_operations/loader.c

lib_stack.o:
	$(CC) $(CCFLAGS) backend/shlib_operations/lib_stack.c
