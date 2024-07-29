#include "../../include/memory/memcache_map.h"
#include "../../include/boss_operations/boss_operations.h"
#include "../../include/memory/cache.h"
#include "../../include/utils/stack.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

Stack_ptr boss_op;

void init_boss_op()
{
    boss_op = create_stack();
}

int cache_write_op
(
    const char *key, 
    const char *message,
    const size_t key_length,
    const size_t message_length
)
{
    if (key_length > MAX_KEY_SIZE)
    {
        return -1;
    }

    // when the operation execution time is added,
    // it will be possible to make a delayed write to the cache

    return cache_write(key, message, message_length, 100000);
}

void print_cache_op(const char *key, const size_t key_length)
{
    if (key_length > MAX_KEY_SIZE)
    {
        return;
    }

    char *message = (char *) cache_read(key, NULL, 0);
    printf("%s\n", message);
}