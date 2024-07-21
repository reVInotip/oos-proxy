#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../../include/utils/hash_map.h"
#include "../../include/guc/guc.h"

// Max count hash keys in hash table
#define COUNT_ELEMENTS_IN_ARRAY 256

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

Values_chain_ptr create_chain_of_values()
{
    return NULL;
}

void push_value_to_chain(Values_chain_ptr *values, void *elem, char *key)
{
    if (*values == NULL)
    {
        *values = (Values_chain_ptr) malloc(sizeof(Values_chain_elem));
        (*values)->next = NULL;
        (*values)->prev = NULL;
        (*values)->value = elem;
        strcpy((*values)->key_str, key);
    }
    else
    {
        Values_chain_ptr curr_link = *values;
        while (curr_link->next != NULL)
        {
            curr_link = curr_link->next;
        }

        curr_link->next = (Values_chain_ptr) malloc(sizeof(Values_chain_elem));
        (curr_link->next)->next = NULL;
        (curr_link->next)->prev = curr_link;
        (curr_link->next)->value = elem;
        strcpy((curr_link->next)->key_str, key);
    }
}

void *get_chain_element(Values_chain_ptr values, const char *key)
{
    Values_chain_ptr curr_link = values;
    while (curr_link != NULL)
    {
        if (!strcmp(curr_link->key_str, key))
        {
            return curr_link->value;
        }

        curr_link = curr_link->next;
    }

    return NULL;
}

void *chain_top(Values_chain_ptr values)
{
    return values->value;
}

void destroy_chain(Values_chain_ptr *values)
{   
    Values_chain_ptr curr_link;
    while (*values != NULL)
    {   
        curr_link = *values;
        *values = (*values)->next;
        free(curr_link);
    }
}

size_t get_chain_size(Values_chain_ptr values)
{   
    Values_chain_ptr curr_link = values;
    size_t size = 0;
    while (curr_link != NULL)
    {   
        ++size;
        curr_link = curr_link->next;
    }

    return size;
}

extern Hash_map_ptr create_map()
{
    Hash_map_ptr map = (Hash_map_ptr) calloc(COUNT_ELEMENTS_IN_ARRAY, sizeof(Hash_map_elem));
    for (size_t i = 0; i < COUNT_ELEMENTS_IN_ARRAY; i++)
    {
        map[i].values = create_chain_of_values();
    }
    
    return map;
}

extern void *get_map_element(Hash_map_ptr map, const char *key)
{   
    int key_index = hash_function(key);
    void *value = NULL;
    size_t count_similar_values = get_chain_size(map[key_index].values);

    if (count_similar_values == 1)
    {
        value = chain_top(map[key_index].values);
    }
    else if (count_similar_values > 1)
    {
        value = get_chain_element(map[key_index].values, key);
    }

    return value;
}

extern void push_to_map(Hash_map_ptr map, char *key, void *value)
{
    int key_index = hash_function(key);
    push_value_to_chain(&map[key_index].values, value, key);
}

extern void destroy_map(Hash_map_ptr *map)
{
    assert(map != NULL);

    for (size_t i = 0; i < get_map_size(*map); i++)
    {
        free((*map)[i].values);
    }
    
    free(*map);
}

extern size_t get_map_size(Hash_map_ptr map)
{
    return COUNT_ELEMENTS_IN_ARRAY;
}