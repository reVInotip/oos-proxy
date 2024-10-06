/**
 * memcache_map.h
 * This file contains structure describing the associative array and methods for it.
 * Implemented based on hash tables. A polynomial hash was used as a hash function.
 * To protect against collisions I use the chain method.
 */
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

#pragma once

// The maximum number of bytes that can contain a key
#define MAX_KEY_SIZE 100

/**
 * \brief This structure implements a linked list needed to resolve collisions.
 * \details TTL and creation time fields are used to check the expiration date of the package.
 * If it is released the package is automatically removed from the cache.
 */
typedef struct collisions_list_elem
{
    // String representation of a key
    char                key_str[MAX_KEY_SIZE];
    void                           *block_ptr; // pointer to cache block with data
    // TTL (Time To Live) and creation time are two values forming priority for queue
    time_t                                TTL;
    time_t                      creation_time;
    struct collisions_list_elem         *next;
    struct collisions_list_elem         *prev;
} Collisions_list_elem;

typedef Collisions_list_elem *Collisions_list_ptr;

typedef struct hash_memmap_elem
{
    Collisions_list_ptr requests;
} Hash_memmap_elem;

typedef Hash_memmap_elem *Hash_memmap_ptr;

// ========= hash functions prototype ==============
extern Hash_memmap_ptr create_memmap();
extern void *get_memmap_element(Hash_memmap_ptr map, const char *key);
extern Collisions_list_elem *push_to_memmap(Hash_memmap_ptr map,
                            const char *key,
                            void *value,
                            time_t TTL,
                            time_t creation_time);
extern void destroy_memmap(Hash_memmap_ptr *map);
extern Collisions_list_ptr *get_memmap_clist(Hash_memmap_ptr map, const char *key);

// ========== collisions list functions prototype ===========
extern void delete_from_collisions_list(Collisions_list_ptr *clist, Collisions_list_elem *del_elem);