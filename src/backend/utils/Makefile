CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

utils.o: 
	$(CC) $(CCFLAGS) $(ROOT)/utils/hash_map.c $(ROOT)/utils/stack.c

utils.a: utils.o
	ar -rc $(OUT)/libutils.a stack.o hash_map.o && ranlib $(OUT)/libutils.a 