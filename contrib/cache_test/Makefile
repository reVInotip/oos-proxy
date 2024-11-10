CC = gcc

CCFLAGS = -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

cache_test: cache_test.o
	$(CC) -shared $(MY_LDFLAGS) $(MAIN_DIR)/contrib/cache_test/cache_test.o -o $(MAIN_DIR)/contrib/cache_test/cache_test.so

cache_test.o:
	$(CC) -I$(INCLUDE_PATH) -fpic -c $(CCFLAGS) $(MAIN_DIR)/contrib/cache_test/cache_test.c -o $(MAIN_DIR)/contrib/cache_test/cache_test.o

clean:
	rm -rf *.o *.a *.so
