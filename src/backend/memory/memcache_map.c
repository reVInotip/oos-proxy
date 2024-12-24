#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "memory/memcache_map.h"
#include "logger/logger.h"
#include "guc/guc.h"
#include "memory/cache_errno.h"

extern enum cache_err_num cache_errno;

// ======== auxiliary structure methods ===========

static Collisions_list_ptr create_collisions_list()
{
    return NULL;
}

static Collisions_list_elem *insert_to_collisions_list(Collisions_list_ptr *clist, const char *key, const Block_t *data)
{   
    Collisions_list_elem *new = (Collisions_list_elem *) malloc(sizeof(Collisions_list_elem));
    assert(new != NULL);

    new->block.block_ptr = data->block_ptr;
    new->block.block_size = data->block_size;
    new->block.creation_time = data->creation_time;
    new->block.TTL = data->TTL;
    new->next = NULL;
    strcpy(new->key_str, key);

    if (*clist == NULL)
    {
        new->prev = NULL;
        *clist = new;
    }
    else
    {
        Collisions_list_elem *last_list_elem = *clist;

        while (last_list_elem->next != NULL)
        {
            ++last_list_elem;
        }

        new->prev = last_list_elem;
        last_list_elem->next = new;
    }

    return new;
}

Collisions_list_elem *get_collisions_list_elem(Collisions_list_ptr clist, const char *key)
{
    while (clist != NULL)
    {
        if (!strcmp(clist->key_str, key))
        {
            return clist;
        }

        clist = clist->next;
    }

    return NULL;
}

void delete_from_collisions_list(Collisions_list_ptr *clist, Collisions_list_elem *del_elem)
{
    if (del_elem->prev != NULL)
    {
        del_elem->prev->next = del_elem->next;
    }
    else
    {
        *clist = del_elem->next;
    }

    if (del_elem->next != NULL)
    {
        del_elem->next->prev = del_elem->prev;
    }

    free(del_elem);
}

void destroy_collisions_list(Collisions_list_ptr *clist)
{   
    Collisions_list_elem *curr_elem = *clist;
    Collisions_list_elem *del_elem = *clist;
    while (*clist != NULL)
    {   
        curr_elem = curr_elem->next;
        delete_from_collisions_list(clist, del_elem);
        del_elem = curr_elem;
    }
}

// ======== cache methods ===========

// Polynomial hash function.
int hash_function(const char *string)
{
    const long mod = 1e9 + 7;
    const int k = 31;
    
    long hash = 0;
    for (int i = 0; string[i] != '\0'; ++i)
    {
        int x = (int) (string[i] - 'a' + 1);
        hash = (hash * k + x) % mod;
    }

    /**
     * The value of a polynomial hash function is mapped
     * to a set to reduce the size of the hash table array.
     */
    return hash % COUNT_ELEMENTS_IN_ARRAY;
}

Hash_memmap_ptr create_memmap()
{
    Hash_memmap_ptr map = (Hash_memmap_ptr) calloc(COUNT_ELEMENTS_IN_ARRAY, sizeof(Hash_memmap_elem));
    assert(map != NULL);

    for (size_t i = 0; i < COUNT_ELEMENTS_IN_ARRAY; i++)
    {
        map[i].requests = create_collisions_list();
    }
    
    return map;
}

/**
 * \brief Get pointer to block from map
 * \return NULL if something went wrong, pointer on memory block if all is OK.
 * To check whether the resulting value is correct, use cache_errno variable.
 * If value is incorrect it was alredy deleted from hash_map.
 */
Block_t *get_memmap_element(Hash_memmap_ptr map, const char *key)
{   
    int key_index = hash_function(key);
    Collisions_list_elem *elem = get_collisions_list_elem(map[key_index].requests, key);
    if (elem == NULL)
    {
        return NULL;
    }

    Block_t *block_ptr = &(elem->block);

    time_t curr_time = time(NULL);
    // check whether the package has expired or its lifetime
    if (curr_time - block_ptr->creation_time > block_ptr->TTL)
    {
        // and delete it if yes
        delete_from_collisions_list(&map[key_index].requests, elem);
        // also set cache_errno variable
        cache_errno = TTL_ELAPSED;
    }

    return block_ptr;
}

/**
 * \brief Get collision list by key
 * \return Returns the entire collisions list with elements by key.
 * The key can be taken from any element of the chain since the hash keys
 * for them are the same, otherwise they would be in different collisions lists.
 * \note The function is used in the LRU algorithm to quickly remove
 * the most recently used elements without a long search for them in the key.
 */
Collisions_list_ptr *get_memmap_clist(Hash_memmap_ptr map, const char *key)
{   
    int key_index = hash_function(key);
    return &map[key_index].requests;
}

/**
 * \brief Push element discribed Block_t structure to map
 * \note Fields from data parameter will be copied, so it can be allocated on stack
 */
Collisions_list_elem *push_to_memmap(Hash_memmap_ptr map, const char *key, const Block_t *data)
{
    assert(key != NULL);
    assert(data != NULL);

    int key_index = hash_function(key);
    return insert_to_collisions_list(&map[key_index].requests, key, data);
}

void destroy_memmap(Hash_memmap_ptr *map)
{
    assert(map != NULL);

    for (size_t i = 0; i < COUNT_ELEMENTS_IN_ARRAY; i++)
    {
        destroy_collisions_list(&(*map)[i].requests);
    }
    
    free(*map);
}
