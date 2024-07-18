CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

utils.o: 
	$(CC) $(CCFLAGS) backend/utils/hash_map.c backend/utils/stack.c

utils.a: utils.o
	ar -rc libutils.a stack.o hash_map.o && ranlib libutils.a