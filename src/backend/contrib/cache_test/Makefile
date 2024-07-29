CC = gcc

CCFLAGS = -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

cache_test: cache_test.o
	$(CC) -shared $(LDFLAGS) cache_test.o -o cache_test.so

cache_test.o:
	$(CC) -fpic -c $(CCFLAGS) cache_test.c

clean:
	rm -rf *.o *.a *.so
