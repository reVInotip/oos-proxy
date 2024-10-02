CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

memory.o:
	$(CC) $(CCFLAGS) $(ROOT)/memory/allocator.c $(ROOT)/memory/cache.c $(ROOT)/memory/LRU_queue.c $(ROOT)/memory/memcache_map.c $(ROOT)/memory/cache_errno.c

memory.a: memory.o
	ar -rc $(OUT)/libmemory.a allocator.o cache.o cache_errno.o LRU_queue.o memcache_map.o && ranlib $(OUT)/libmemory.a