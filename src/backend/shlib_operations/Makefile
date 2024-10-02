CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

shlib_operations.o:
	$(CC) $(CCFLAGS) $(ROOT)/shlib_operations/loader.c $(ROOT)/shlib_operations/destroyer.c $(ROOT)/shlib_operations/initializer.c $(ROOT)/shlib_operations/exec_requests.c

shlib_operations.a: shlib_operations.o
	ar -rc $(OUT)/liboperations.a exec_requests.o initializer.o loader.o destroyer.o && ranlib $(OUT)/liboperations.a