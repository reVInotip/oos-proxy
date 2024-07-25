#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "../../include/memory/memcache_map.h"
#include "../../include/guc/guc.h"

// Max count hash keys in hash table
#define COUNT_ELEMENTS_IN_ARRAY 256 // 1024

#define PQUEUE_SIZE 8

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

/**
 * \brief first elem is child and second is parent
 */
void swap(Priority_queue_elem *child, Priority_queue_elem *parent)
{
    Priority_queue_elem tmp = *child;
    *child = *parent;
    *parent = tmp;
}

time_t get_priority(Priority_queue_elem *elem)
{
    return elem->creation_time + elem->TTL;
}

void shift_up(Pqueue_ptr pqueue, int i)
{
    while (get_priority(&pqueue->data[i]) > get_priority(&pqueue->data[(i - 1) / 2]))
    {
        swap(&(pqueue->data[i]), &(pqueue->data[(i - 1) / 2]));
        i = (i - 1) / 2;
    }
}

void shift_down(Pqueue_ptr pqueue, int i)
{   
    int left = 0;
    int right = 0;
    int j = 0;
    while (2 * i + 1 < pqueue->curr_size)
    {
        left = 2 * i + 1;
        right = 2 * i + 2;
        j = left;
        if (right < pqueue->curr_size &&
            get_priority(&pqueue->data[right]) > get_priority(&pqueue->data[left]))
        {
            j = right;
        }

        if (get_priority(&pqueue->data[i]) >= get_priority(&pqueue->data[j]))
        {
            break;
        }

        swap(&pqueue->data[i], &pqueue->data[j]);
        i = j;
    }
}

Pqueue_ptr create_pquque()
{   
    Pqueue_ptr pqueue = (Pqueue_ptr) malloc(sizeof(Pqueue));
    assert(pqueue != NULL);

    pqueue->data = (Priority_queue_ptr) malloc(sizeof(Priority_queue_elem) * PQUEUE_SIZE);
    assert(pqueue->data != NULL);

    pqueue->curr_size = 0;
    pqueue->size = PQUEUE_SIZE;

    return pqueue;
}

void reallocate_pqueue(Pqueue_ptr pqueue)
{   
    pqueue->data = (Priority_queue_ptr) realloc(pqueue->data,
                                                pqueue->size * sizeof(Priority_queue_elem) * 2);
    assert(pqueue->data != NULL);

    pqueue->size *= 2;
}

void insert_value_to_pqueue(Pqueue_ptr pqueue, void *value, char *key, time_t TTL, time_t creation_time)
{
    ++pqueue->curr_size;
    if (pqueue->curr_size > pqueue->size)
    {
        reallocate_pqueue(pqueue);
    }

    strcpy(pqueue->data[pqueue->curr_size - 1].key_str, key);
    pqueue->data[pqueue->curr_size - 1].TTL = TTL;
    pqueue->data[pqueue->curr_size - 1].creation_time = creation_time;

    shift_up(pqueue, pqueue->curr_size - 1);
}

void delete_value_from_pqueue(Pqueue_ptr pqueue, int i)
{
    pqueue->data[i] = pqueue->data[pqueue->curr_size - 1];
    --pqueue->curr_size;
    shift_down(pqueue, i);
}

int get_pqueue_element(Pqueue_ptr pqueue, const char *key)
{
    for (size_t i = 0; i < pqueue->curr_size; i++)
    {
        if (!strcmp((pqueue->data[i]).key_str, key))
        {
            return (int) i;
        }
    }

    return -1;
}

void destroy_pqueue(Pqueue_ptr pqueue)
{   
    free(pqueue->data);
    free(pqueue);
}

size_t get_pqueue_size(Pqueue_ptr pqueue)
{   
    return pqueue->curr_size;
}

extern Hash_memmap_ptr create_memmap()
{
    Hash_memmap_ptr map = (Hash_memmap_ptr) calloc(COUNT_ELEMENTS_IN_ARRAY, sizeof(Hash_memmap_elem));
    assert(map != NULL);

    for (size_t i = 0; i < COUNT_ELEMENTS_IN_ARRAY; i++)
    {
        map[i].requests = create_pquque();
    }
    
    return map;
}

/**
 * \brief Get pointer to block or chain from map
 * \param [out] is_value_invalid - check is TTL elapsed or not
 * \return NULL if something went wrong or value is invalid, ptr if all is OK
 */
extern void *get_memmap_element(Hash_memmap_ptr map, const char *key, bool *is_value_invalid)
{   
    int key_index = hash_function(key);
    int elem_index = get_pqueue_element(map[key_index].requests, key);
    if (elem_index == -1)
    {
        return NULL;
    }

    time_t curr_time = time(NULL);
    *is_value_invalid = curr_time - map[key_index].requests->data[elem_index].creation_time >
                                    map[key_index].requests->data[elem_index].TTL;

    if (*is_value_invalid)
    {
        return NULL;
    }

    return map[key_index].requests->data[elem_index].block_ptr;
}

/**
 * \brief Get pointer to block or chain from map
 * \param [out] is_value_invalid - check is TTL elapsed or not
 * \return NULL if something went wrong or value is invalid, ptr if all is OK
 */
extern void *get_memmap_top(Hash_memmap_ptr map, const char *key, bool *is_value_invalid)
{
    int key_index = hash_function(key);

    if (map[key_index].requests->curr_size == 0)
    {
        return NULL;
    }

    time_t curr_time = time(NULL);
    *is_value_invalid = curr_time - map[key_index].requests->data[0].creation_time >
                                    map[key_index].requests->data[0].TTL;
    if (*is_value_invalid)
    {
        return NULL;
    }

    return map[key_index].requests->data[0].block_ptr;
}

extern void delete_memmap_element(Hash_memmap_ptr map, const char *key)
{
    int key_index = hash_function(key);
    int elem_index = get_pqueue_element(map[key_index].requests, key);
    if (elem_index == -1)
    {
        return NULL;
    }

    delete_value_from_pqueue(map[key_index].requests, elem_index);
}

extern void clear_memmap_by_key(Hash_memmap_ptr map, const char *key)
{
    int key_index = hash_function(key);
    map[key_index].requests->curr_size = 0;
}

extern void push_to_memmap(Hash_memmap_ptr map, char *key, void *value, time_t TTL, time_t creation_time)
{
    int key_index = hash_function(key);
    insert_value_to_pqueue(map[key_index].requests, value, key, TTL, creation_time);
}

extern void destroy_memmap(Hash_memmap_ptr *map)
{
    assert(map != NULL);

    for (size_t i = 0; i < get_map_size(*map); i++)
    {
        destroy_pqueue((*map)[i].requests);
    }
    
    free(*map);
}

extern size_t get_memmap_size(Hash_memmap_ptr map)
{
    return COUNT_ELEMENTS_IN_ARRAY;
}