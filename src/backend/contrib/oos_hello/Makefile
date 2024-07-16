CC = gcc

CCFLAGS = -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

oos_hello: oos_hello.o
	$(CC) -shared $(LDFLAGS) oos_hello.o -o oos_hello.so

oos_hello.o:
	$(CC) -fpic -c $(CCFLAGS) oos_hello.c
