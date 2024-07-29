/**
 * hash_map.h
 * Structure describing the associative array and methods for it.
 * Implemented based on hash tables. A polynomial hash was used as a hash function.
 * To protect against collisions and add multiple values ​​with the same key, a priority
 * queue of a not fixed size is used (this structure was chosen to support working with the cache).
 */
#include <stdlib.h>
#include "stack.h"

#pragma once

// The maximum number of bytes that can contain a key
#define MAX_KEY_SIZE 100

/**
 * \brief Structure describing the priority queue used to prevent collisions and
 *  add multiple values with the same key
 * \todo change in to double linked list
 */
typedef struct priority_queue_elem
{   
    // String representation of a key
    char key_str[MAX_KEY_SIZE];
    void *value;
    int priority;
} Priority_queue_elem;

typedef Priority_queue_elem *Priority_queue_ptr;

typedef struct pqueue
{
    size_t curr_size;
    size_t size;
    Priority_queue_ptr data;
} Pqueue;

typedef Pqueue *Pqueue_ptr;

typedef struct hash_map_elem
{
    Pqueue_ptr values;
} Hash_map_elem;

typedef Hash_map_elem *Hash_map_ptr;

// Use if you don`t need priority
#define push_to_map(map, key, value) \
    do \
    { \
        push_to_map_with_priority(map, key, value, 0); \
    } while (0)

extern Hash_map_ptr create_map();
extern void *get_map_element(Hash_map_ptr map, const char *key);
extern void push_to_map_with_priority(Hash_map_ptr map, char *key, void *value, int priority);
extern void destroy_map(Hash_map_ptr *map);
extern size_t get_map_size(Hash_map_ptr map);