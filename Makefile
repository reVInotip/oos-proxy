CC = gcc
CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2
LDFLAGS = -ldl
LDFLAGS_DEBUG = -fsanitize=address,leak,undefined
SOURCE = install
ROOT = src/backend
INCLUDE_DIR = src/include
BIN_NAME = proxy
OBJ = bg_worker.o boss_operations.o guc.o
MAIN_OBJ = main.o
LIBS_LINK = -lutils -loperations -lmemory -llogger
LIBS = utils.a operations.a memory.a logger.so

all:
	make clean_source_dir
	make create_source_dir
	make link
	echo "Done"

debug:
	make clean_source_dir
	make create_source_dir
	make link_debug
	echo "Done"

link: $(MAIN_OBJ) $(OBJ)
	$(CC) $(LDFLAGS) $^ -L$(SOURCE) $(LIBS_LINK) -Wl,-rpath=$(SOURCE) -o $(SOURCE)/$(BIN_NAME)

link_debug: $(MAIN_OBJ) $(OBJ)
	$(CC) $(LDFLAGS) $(LDFLAGS_DEBUG) $^ -L$(SOURCE) $(LIBS_LINK) -Wl,-rpath=$(SOURCE) -o $(SOURCE)/$(BIN_NAME)

include $(ROOT)/Makefile

create_source_dir:
	mkdir install

clean_source_dir:
	rm -rf install

clean_obj:
	rm -rf *.o

clean: clean_source_dir clean_obj
	echo "Full clean"