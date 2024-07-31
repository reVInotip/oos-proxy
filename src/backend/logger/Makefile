CC = gcc

CCFLAGS = -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

logger.o:
	$(CC) -fpic -c $(CCFLAGS) backend/logger/logger.c

logger.so: logger.o
	$(CC) -shared $(LDFLAGS) logger.o -o liblogger.so