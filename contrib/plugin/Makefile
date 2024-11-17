CC = gcc

MY_CCFLAGS = -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

plugin: plugin.o
	$(CC) -shared $(MY_LDFLAGS) $(MAIN_DIR)/contrib/plugin/plugin.o -o $(MAIN_DIR)/contrib/plugin/plugin.so

plugin.o:
	$(CC) -I$(INCLUDE_PATH) -fpic -c $(MY_CCFLAGS) $(MAIN_DIR)/contrib/plugin/plugin.c -o $(MAIN_DIR)/contrib/plugin/plugin.o

clean:
	rm -rf *.o *.a *.so
