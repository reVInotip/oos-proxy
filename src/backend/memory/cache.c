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
#include "../../include/memory/LRU_queue.h"
#include "../../include/memory/cache_errno.h"
#include "../../include/memory/cache.h"

Hash_memmap_ptr memcache;
Pqueue_ptr LRU_queue;

extern enum cache_err_num cache_errno;

/**
 * \brief Initialize cache and allocator for it
 */
extern void init_cache()
{
    init_OOS_allocator();
    memcache = create_memmap();
    LRU_queue = create_pquque();
}

extern void drop_cache()
{
    destroy_OOS_allocator();
    destroy_memmap(&memcache);
    destroy_pqueue(LRU_queue);
}

/**
 * \brief Clearing cache when full using LRU alogrithm
 * \details The function will clear the cache until a new element can be inserted into it.
 * \param [in] clearing_size - number of bytes to clear.
 * \warning If the algorithm does not work, the function terminates the program urgently.
 */
void clear_cache(const size_t clearing_size)
{
    while (true)
    {
        Collisions_list_elem *elem = extract_min(LRU_queue);
        if (elem == NULL)
        {
            elog(ERROR, "Cache overflow and can`t be cleaned");
            abort(); // Come up with something smarter please ;) (For example you can use signal interrupt handler)
        }
        if (check_is_mem_availiable(clearing_size))
        {
            break;
        }

        Collisions_list_ptr *clist_for_curr_key = get_memmap_clist(memcache, elem->key_str);
        if (*clist_for_curr_key == NULL)
        {
            elog(ERROR, "Cache overflow and can`t be cleaned");
            abort(); // Come up with something smarter please ;) (For example you can use signal interrupt handler)
        }

        OOS_free(elem->block_ptr);
        delete_from_collisions_list(clist_for_curr_key, elem);
    }
}

/**
 * \brief Write something to cache
 * \param [in] key - key that will be used to recording
 * \param [in] message - message for writing in cache
 * \param [in] len - message len in bytes
 * \param [in] TTL - Time To Live: time during which the message is correct
 * \return -1 if something went wrong, 0 if all is OK
 */
extern int cache_write(const char *key, const char *message, const size_t len, unsigned TTL)
{   
    void *addr = OOS_allocate(len);
    if (addr == NULL)
    {
        if (cache_errno == NO_FREE_SPACE)
        {
            clear_cache(len);

            // Here we can be sure that there will be enough space in the cache
            // due to the specifics of the function cache_clear()
            addr = OOS_allocate(len);
            assert(addr != NULL);
        }
        else
        {
            elog(ERROR, "Allocation error");
            return -1;
        }
    }

    time_t t = time(NULL);
    if (t == (time_t) -1)
    {
        elog(ERROR, "time error: %s", strerror(errno));
        return -1;
    }

    Collisions_list_elem *elem = push_to_memmap(memcache, key, addr, TTL, t);
    insert_value_to_pqueue(LRU_queue, elem);
    memcpy(addr, message, len);

    return 0;
}

/**
 * \brief Read something from cache
 * \details If TTL of message ran out the message will be deleted and function return 1
 * \param [in] key - key that will be used to recording
 * \param [in] buffer - buffer for reading from cache data (if NULL function return pointer to data in cache)
 * \param [in] len - message len in bytes
 * \param [in] TTL - Time To Live: time during which the message is correct
 * \return (void *)-1 (ERR) if something went wrong, (void *)0 or pointer to cache block if all is OK,
 * (void *) -2 (TTL_ERR) if TTL is elapsed and message was deleted
 */
extern void *cache_read(const char *key, char *buffer, const size_t len)
{
    // check if curr value invalid
    void *addr = get_memmap_element(memcache, key);
    if (addr == NULL)
    {
        elog(ERROR, "Element with key: %s not found in cache", key);
        return ERR;
    }

    if (cache_errno == TTL_ELAPSED)
    {
        delete_from_pqueue(LRU_queue, addr);
        OOS_free(addr);
        return TTL_ERR;
    }

    update_element_time(LRU_queue, addr);

    if (buffer == NULL)
    {
        return addr;
    }

    memcpy(buffer, addr, len);
    return (void *) 0;
}
