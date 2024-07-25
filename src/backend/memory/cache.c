#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include "../../include/memory/memcache_map.h"
#include "../../include/memory/allocator.h"
#include "../../include/logger/logger.h"

Hash_memmap_ptr memcache;

void init_cache()
{
    init_OOS_allocator();
    memcache = create_memmap();
}

/**
 * \brief Write something to cache
 * \param [in] key - key that will be used to recording
 * \param [in] message - message for writing in cache
 * \param [in] len - message len in bytes
 * \param [in] TTL - Time To Live: time during which the message is correct
 * \return -1 if something went wrong, 0 if all is OK
 */
int cache_write(const char *key, const char *message, const size_t len, unsigned TTL)
{
    void *addr = OOS_allocate(len / MAX_BLOCK_SIZE + (len % MAX_BLOCK_SIZE > 0));
    assert(addr != NULL);

    time_t t = time(NULL);
    if (t == (time_t) -1)
    {
        elog(ERROR, "time error: %s", strerror(errno));
        return -1;
    }

    push_to_memmap(memcache, key, TTL, addr, t);

    return 0;
}

/**
 * \brief Read something from cache
 * \details If TTL of message ran out the message will be deleted and function return 1
 * \param [in] key - key that will be used to recording
 * \param [in] message - message for writing in cache
 * \param [in] len - message len in bytes
 * \param [in] TTL - Time To Live: time during which the message is correct
 * \return -1 if something went wrong, 0 if all is OK, 1 if TTL is elapsed and message was deleted
 */
int cache_read(const char *key, const char *buffer, const size_t len)
{
    bool is_ptr_invalid = false;

    // check if all values invalid
    void *addr = get_memmap_top(memcache, key, &is_ptr_invalid);
    if (addr == NULL)
    {
        if (is_ptr_invalid)
        {
            clear_memmap_by_key(memcache, key);
            return 1;
        }
        return -1;
    }
    
    // check if curr value invalid
    addr = get_memmap_element(memcache, key, &is_ptr_invalid);
    if (addr == NULL)
    {
        if (is_ptr_invalid)
        {
            delete_memmap_element(memcache, key);
            return 1;
        }
        return -1;
    }

    int is_err = OOS_read(addr, buffer, len);
    if (is_err < 0)
    {
        elog(ERROR, "Cache read error");
        return -1;
    }

    return 0;
}
