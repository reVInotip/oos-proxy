/**
 * memcache_map.h
 * Structure describing the associative array and methods for it.
 * Implemented based on hash tables. A polynomial hash was used as a hash function.
 * To protect against collisions and add multiple values ​​with the same key, a priority
 * queue of a not fixed size is used (this structure was chosen to support working with the cache).
 */

#pragma once

#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"

// The maximum number of bytes that can contain a key
#define MAX_KEY_SIZE 100

/**
 * \brief Structure describing the priority queue used to prevent collisions and
 *  add multiple values with the same key
 */
typedef struct priority_queue_elem
{   
    // String representation of a key
    char key_str[MAX_KEY_SIZE];
    void            *block_ptr;
    time_t                 TTL; // --\ 
                                //    |--- two value forming priority for queue
    time_t       creation_time; // --/
} Priority_queue_elem;

typedef Priority_queue_elem *Priority_queue_ptr;

typedef struct pqueue
{
    size_t curr_size;
    size_t size;
    Priority_queue_ptr data;
} Pqueue;

typedef Pqueue *Pqueue_ptr;

typedef struct hash_memmap_elem
{
    Pqueue_ptr requests;
} Hash_memmap_elem;

typedef Hash_memmap_elem *Hash_memmap_ptr;

extern Hash_memmap_ptr create_memmap();
extern void *get_memmap_element(Hash_memmap_ptr map, const char *key, bool *is_value_invalid);
extern void push_to_memmap(Hash_memmap_ptr map,
                            char *key,
                            void *value,
                            time_t TTL,
                            time_t creation_time);
extern void clear_memmap_by_key(Hash_memmap_ptr map, const char *key);
extern void delete_memmap_element(Hash_memmap_ptr map, const char *key);
extern void *get_memmap_top(Hash_memmap_ptr map, const char *key, bool *is_value_invalid);
extern void destroy_memmap(Hash_memmap_ptr *map);
extern size_t get_memmap_size(Hash_memmap_ptr map);
