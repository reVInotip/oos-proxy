CC = gcc

CCFLAGS = -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

log_copy: log_copy.o
	$(CC) -shared $(LDFLAGS) $(MAIN_DIR)/contrib/log_copy/log_copy.o -L$(SOURCE) -llogger -Wl,-rpath=$(SOURCE) -o $(MAIN_DIR)/contrib/log_copy/log_copy.so

log_copy.o:
	$(CC) -I$(INCLUDE_PATH) -fpic -c $(CCFLAGS) $(MAIN_DIR)/contrib/log_copy/log_copy.c -o $(MAIN_DIR)/contrib/log_copy/log_copy.o

clean:
	rm -rf *.o *.a *.so