CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

main.o: utils.a shlib_operations.a logger.o guc.o
	$(CC) $(CCFLAGS) backend/main.c

include backend/shlib_operations/Makefile
include backend/utils/Makefile
include backend/guc/Makefile
include backend/logger/Makefile