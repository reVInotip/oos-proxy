CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

LDFLAGS = -ldl

all: main.o utils.a shlib_operations.a logger.o guc.o
	$(CC) $(LDFLAGS) guc.o logger.o loader.o main.o -L . libutils.a liboperations.a -o oos_proxy

include backend/Makefile

clean:
	rm -rf *.o *.a