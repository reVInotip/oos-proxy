CC = gcc

CCFLAGS = -fpic -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2

memory.o:
	$(CC) $(CCFLAGS) backend/memory/allocator.c backend/memory/cache.c backend/memory/LRU_queue.c backend/memory/memcache_map.c backend/memory/cache_errno.c

memory.a: memory.o
	ar -rc libmemory.a allocator.o cache.o cache_errno.o LRU_queue.o memcache_map.o && ranlib libmemory.a