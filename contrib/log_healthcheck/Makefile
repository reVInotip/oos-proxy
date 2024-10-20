CC = gcc

CCFLAGS = -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

log_healthcheck: log_healthcheck.o
	$(CC) -shared $(LDFLAGS) $(MAIN_DIR)/contrib/log_healthcheck/log_healthcheck.o -L$(SOURCE) -llogger -Wl,-rpath=$(SOURCE) -o $(MAIN_DIR)/contrib/log_healthcheck/log_healthcheck.so

log_healthcheck.o:
	$(CC) -I$(INCLUDE_PATH) -fpic -c $(CCFLAGS) $(MAIN_DIR)/contrib/log_healthcheck/log_healthcheck.c -o $(MAIN_DIR)/contrib/log_healthcheck/log_healthcheck.o

clean:
	rm -rf *.o *.a *.so

