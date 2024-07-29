#include <stdlib.h>

#pragma once

#define ERR ((void *) -1)
#define TTL_ERR ((void *) -2)

extern void *cache_read(const char *key, char *buffer, const size_t len);
extern int cache_write(const char *key, const char *message, const size_t len, unsigned TTL);
extern void init_cache();