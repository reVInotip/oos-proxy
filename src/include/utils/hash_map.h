/**
 * hash_map.h
 * Structure describing the associative array and methods for it.
 * Implemented based on hash tables. A polynomial hash was used as a hash function.
 * The chain method is used to prevent collisions. They are implemented based on a doubly
 * linked list.
 */

#pragma once

#include <stdlib.h>
#include "stack.h"


// The maximum number of bytes that can contain a key
#define MAX_KEY_SIZE 100

// Structure describing the chains used to prevent collisions
typedef struct values_chain_elem
{   
    // String representation of a key
    char key_str[MAX_KEY_SIZE];
    void *value;
    struct values_chain_elem *prev;
    struct values_chain_elem *next;
} Values_chain_elem;

typedef Values_chain_elem *Values_chain_ptr;

typedef struct hash_map_elem
{
    Values_chain_ptr values;
} Hash_map_elem;

typedef Hash_map_elem *Hash_map_ptr;

extern Hash_map_ptr create_map();
extern void *get_map_element(Hash_map_ptr map, const char *key);
extern void push_to_map(Hash_map_ptr map, char *key, void *value);
extern void destroy_map(Hash_map_ptr *map);
extern size_t get_map_size(Hash_map_ptr map);