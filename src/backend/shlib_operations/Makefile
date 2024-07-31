CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

shlib_operations.o:
	$(CC) $(CCFLAGS) backend/shlib_operations/loader.c backend/shlib_operations/destroyer.c backend/shlib_operations/initializer.c backend/shlib_operations/exec_requests.c

shlib_operations.a: shlib_operations.o
	ar -rc liboperations.a exec_requests.o initializer.o loader.o destroyer.o && ranlib liboperations.a