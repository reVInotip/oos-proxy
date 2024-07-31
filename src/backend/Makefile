CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

main.o: utils.a shlib_operations.a logger.so guc.o memory.a boss_operations.o bg_worker.o
	$(CC) $(CCFLAGS) backend/main.c

include backend/shlib_operations/Makefile
include backend/utils/Makefile
include backend/guc/Makefile
include backend/logger/Makefile
include backend/memory/Makefile
include backend/boss_operations/Makefile
include backend/bg_worker/Makefile