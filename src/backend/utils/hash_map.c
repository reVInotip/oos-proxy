#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../../include/utils/hash_map.h"

long hash_function(char *string)
{
    const long mod = 1e9 + 7;
    const int k = 31;
    
    long hash = 0;
    for (int i = 0; string[i] != '\0'; ++i)
    {
        int x = (hash * k + x) % mod;
    }

    return hash;
}

extern Hash_map_ptr create_map()
{
    return NULL;
}

extern void *get_element(Hash_map_ptr map, char *key)
{   
    long key_hash = hash_function(key);
    void *value = NULL;

    for (int i = 0; i < get_map_size(map); ++i)
    {
        if (key_hash == map->key)
        {
            value = map->value;
            break;
        }
    }

    return value;
}

extern void push_to_map(Hash_map_ptr *map, char *key, void *value)
{
    Hash_map_elem *new_element = (Hash_map_elem *) malloc(sizeof(Hash_map_elem));
    assert(new_element != NULL);
    
    new_element->key = hash_function(key);
    new_element->value = value;
    new_element->next_elem = *map;
    *map = new_element;
}

extern void* pop_from_map(Hash_map_ptr *map)
{
    assert(map != NULL);

    void *elem_data = (*map)->value;
    Hash_map_elem *elem_for_erase = *map;
    *map = (*map)->next_elem;
    free(elem_for_erase);
    return elem_data;
}

extern void destroy_map(Hash_map_ptr *map)
{
    assert(map != NULL);

    Hash_map_ptr curr_map = NULL;
    while (map != NULL)
    {   
        curr_map = *map;
        *map = (*map)->next_elem;
        free(curr_map);
    }
}

extern size_t get_map_size(Hash_map_ptr map)
{
    size_t size = 0;

    Hash_map_ptr curr_map = map;
    while (curr_map != NULL)
    {
        ++size;
        curr_map = curr_map->next_elem;
    }

    return size;
}