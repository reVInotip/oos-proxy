memory.o:
	$(CC) $(CCFLAGS) $(ROOT)/memory/allocator.c $(ROOT)/memory/cache.c $(ROOT)/memory/LRU_queue.c $(ROOT)/memory/memcache_map.c $(ROOT)/memory/cache_errno.c

memory.a: memory.o
	ar -rc $(SOURCE)/libmemory.a allocator.o cache.o cache_errno.o LRU_queue.o memcache_map.o && ranlib $(SOURCE)/libmemory.a