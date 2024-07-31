CC = gcc

CCFLAGS = -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

log_healthcheck: log_healthcheck.o
	$(CC) -shared $(LDFLAGS) log_healthcheck.o -L/home/grisha/Projects/oos-proxy/src -llogger -Wl,-rpath=/home/grisha/Projects/oos-proxy/src -o log_healthcheck.so

log_healthcheck.o:
	$(CC) -fpic -c $(CCFLAGS) log_healthcheck.c

clean:
	rm -rf *.o *.a *.so

