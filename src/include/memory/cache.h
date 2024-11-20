#include <stdlib.h>

#pragma once

typedef struct cache_data_t
{
    void   *ptr; // pointer to data in cache
    size_t size; // size of this data in bytes
} Cache_data_t;

#define ERR ((void *) -1)
#define TTL_ERR ((void *) -2)

extern Cache_data_t cache_read(const char *key, char *buffer, const size_t buffer_size);
extern int cache_write(const char *key, const char *message, const size_t len, unsigned TTL);
extern void init_cache();
extern void drop_cache();