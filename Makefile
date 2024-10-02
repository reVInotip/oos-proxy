CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

LDFLAGS = -ldl

LDFLAGS_DEBUG = -fsanitize=address,leak,undefined

OUT = install

ROOT = src/backend

BIN_NAME = proxy

all: main.o utils.a shlib_operations.a logger.so guc.o memory.a boss_operations.o bg_worker.o
	$(CC) $(LDFLAGS) bg_worker.o boss_operations.o guc.o loader.o main.o -L . $(OUT)/libutils.a $(OUT)/liboperations.a $(OUT)/libmemory.a $(OUT)/liblogger.so -Wl,-rpath= -o $(OUT)/$(BIN_NAME)

debug: main.o utils.a shlib_operations.a logger.so guc.o memory.a boss_operations.o bg_worker.o
	$(CC) $(LDFLAGS) $(LDFLAGS_DEBUG) bg_worker.o boss_operations.o guc.o loader.o main.o -L . libutils.a liboperations.a libmemory.a liblogger.so -o  $(OUT)/$(BIN_NAME)

include $(ROOT)/Makefile

clean:
	rm -rf *.o *.a *.so