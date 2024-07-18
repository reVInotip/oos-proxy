#pragma once

#include <stdlib.h>

typedef struct hash_map_elem
{
    long key;
    void *value;
    struct hash_map_elem *next_elem;   
} Hash_map_elem;

typedef Hash_map_elem *Hash_map_ptr;

extern Hash_map_ptr create_map();
extern void *get_map_element(Hash_map_ptr map, const char *key);
extern void push_to_map(Hash_map_ptr *map, const char *key, void *value);
extern void* pop_from_map(Hash_map_ptr *map);
extern void destroy_map(Hash_map_ptr *map);
extern size_t get_map_size(Hash_map_ptr map);