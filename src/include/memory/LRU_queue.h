/**
 * LRU_queue.h
 * This file contians strcuts and operations necessary for the LRU algorithm to work.
 * Minimal heap (priority queue) is used to retrieve the most recently used data from the cache.
 */
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include "memcache_map.h"

#pragma once

/**
 * \brief Structure describing the priority queue used to LRU algorithm
 */
typedef struct priority_queue_elem
{   
    Collisions_list_elem *map_elem_ptr; //corresponding element in hash map
    time_t             last_using_time;
} Priority_queue_elem;

typedef Priority_queue_elem *Priority_queue_ptr;

/**
 * \brief Meta-data for priority queue and pointer on it
 */
typedef struct pqueue
{
    size_t        curr_size;
    size_t             size;
    Priority_queue_ptr data;
} Pqueue;

typedef Pqueue *Pqueue_ptr;

extern Pqueue_ptr create_pquque();
extern void reallocate_pqueue(Pqueue_ptr pqueue);
extern void insert_value_to_pqueue(Pqueue_ptr pqueue, Collisions_list_elem *map_elem_ptr);
extern Collisions_list_elem *extract_min(Pqueue_ptr pqueue);
extern int get_pqueue_element(Pqueue_ptr pqueue, void *block_ptr);
extern void destroy_pqueue(Pqueue_ptr pqueue);
extern void delete_from_pqueue(Pqueue_ptr pqueue, void *block_ptr);
extern void update_element_time(Pqueue_ptr pqueue, void *block_ptr);