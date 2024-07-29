#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../../include/utils/hash_map.h"
#include "../../include/guc/guc.h"

// Max count hash keys in hash table
#define COUNT_ELEMENTS_IN_ARRAY 256

#define PQUEUE_SIZE 8

// Polynomial hash function.
static int hash_function(const char *string)
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

/**
 * \brief first elem is child and second is parent
 */
static void swap(Priority_queue_elem *child, Priority_queue_elem *parent)
{
    Priority_queue_elem tmp = *child;
    *child = *parent;
    *parent = tmp;
}

static void shift_up(Pqueue_ptr pqueue, int i)
{
    while ((pqueue->data[i]).priority < (pqueue->data[(i - 1) / 2]).priority)
    {
        swap(&(pqueue->data[i]), &(pqueue->data[(i - 1) / 2]));
        i = (i - 1) / 2;
    }
}

static Pqueue_ptr create_pquque()
{   
    Pqueue_ptr pqueue = (Pqueue_ptr) malloc(sizeof(Pqueue));
    assert(pqueue != NULL);

    pqueue->data = (Priority_queue_ptr) malloc(sizeof(Priority_queue_elem) * PQUEUE_SIZE);
    assert(pqueue->data != NULL);

    pqueue->curr_size = 0;
    pqueue->size = PQUEUE_SIZE;

    return pqueue;
}

static void reallocate_pqueue(Pqueue_ptr pqueue)
{   
    pqueue->data = (Priority_queue_ptr) realloc(pqueue->data,
                                                pqueue->size * sizeof(Priority_queue_elem) * 2);
    assert(pqueue->data != NULL);

    printf("Here\n");

    pqueue->size *= 2;
}

static void insert_value_to_pqueue(Pqueue_ptr pqueue, void *value, char *key, int priority)
{
    ++pqueue->curr_size;
    if (pqueue->curr_size > pqueue->size)
    {
        reallocate_pqueue(pqueue);
    }

    strcpy(pqueue->data[pqueue->curr_size - 1].key_str, key);
    pqueue->data[pqueue->curr_size - 1].priority = priority;
    pqueue->data[pqueue->curr_size - 1].value = value;

    shift_up(pqueue, pqueue->curr_size - 1);
}

static void *get_pqueue_element(Pqueue_ptr pqueue, const char *key)
{
    for (size_t i = 0; i < pqueue->curr_size; i++)
    {
        if (!strcmp((pqueue->data[i]).key_str, key))
        {
            return (pqueue->data[i]).value;
        }
    }

    return NULL;
}

static void *pqueue_min(Pqueue_ptr pqueue)
{
    return pqueue->data[0].value;
}

static void destroy_pqueue(Pqueue_ptr pqueue)
{   
    free(pqueue->data);
    free(pqueue);
}

static size_t get_pqueue_size(Pqueue_ptr pqueue)
{   
    return pqueue->curr_size;
}

extern Hash_map_ptr create_map()
{
    Hash_map_ptr map = (Hash_map_ptr) calloc(COUNT_ELEMENTS_IN_ARRAY, sizeof(Hash_map_elem));
    assert(map != NULL);

    for (size_t i = 0; i < COUNT_ELEMENTS_IN_ARRAY; i++)
    {
        map[i].values = create_pquque();
    }
    
    return map;
}

extern void *get_map_element(Hash_map_ptr map, const char *key)
{   
    int key_index = hash_function(key);
    void *value = NULL;
    size_t count_similar_values = get_pqueue_size(map[key_index].values);

    if (count_similar_values == 1)
    {
        value = pqueue_min(map[key_index].values);
    }
    else if (count_similar_values > 1)
    {
        value = get_pqueue_element(map[key_index].values, key);
    }

    return value;
}

extern void push_to_map_with_priority(Hash_map_ptr map, char *key, void *value, int priority)
{
    int key_index = hash_function(key);
    insert_value_to_pqueue(map[key_index].values, value, key, priority);
}

extern void destroy_map(Hash_map_ptr *map)
{
    assert(map != NULL);

    for (size_t i = 0; i < get_map_size(*map); i++)
    {
        destroy_pqueue((*map)[i].values);
    }
    
    free(*map);
}

extern size_t get_map_size(Hash_map_ptr map)
{
    return COUNT_ELEMENTS_IN_ARRAY;
}