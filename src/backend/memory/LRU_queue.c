#include <time.h>
#include <assert.h>
#include "../../include/memory/LRU_queue.h"
#include "../../include/memory/memcache_map.h"

#define PQUEUE_START_SIZE 1024

/**
 * \brief first elem is child and second is parent
 */
static void swap(Priority_queue_elem *child, Priority_queue_elem *parent)
{
    Priority_queue_elem tmp = *child;
    *child = *parent;
    *parent = tmp;
}

static inline time_t get_priority(Priority_queue_elem *elem)
{
    return elem->last_using_time;
}

static void shift_up(Pqueue_ptr pqueue, int i)
{
    while (get_priority(&pqueue->data[i]) < get_priority(&pqueue->data[(i - 1) / 2]))
    {
        swap(&(pqueue->data[i]), &(pqueue->data[(i - 1) / 2]));
        i = (i - 1) / 2;
    }
}

static void shift_down(Pqueue_ptr pqueue, int i)
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
            get_priority(&pqueue->data[right]) < get_priority(&pqueue->data[left]))
        {
            j = right;
        }

        if (get_priority(&pqueue->data[i]) <= get_priority(&pqueue->data[j]))
        {
            break;
        }

        swap(&pqueue->data[i], &pqueue->data[j]);
        i = j;
    }
}

extern Pqueue_ptr create_pquque()
{   
    Pqueue_ptr pqueue = (Pqueue_ptr) malloc(sizeof(Pqueue));
    assert(pqueue != NULL);

    pqueue->data = (Priority_queue_ptr) malloc(sizeof(Priority_queue_elem) * PQUEUE_START_SIZE);
    assert(pqueue->data != NULL);

    pqueue->curr_size = 0;
    pqueue->size = PQUEUE_START_SIZE;

    return pqueue;
}

extern void reallocate_pqueue(Pqueue_ptr pqueue)
{   
    pqueue->data = (Priority_queue_ptr) realloc(pqueue->data,
                                                pqueue->size * sizeof(Priority_queue_elem) * 2);
    assert(pqueue->data != NULL);

    pqueue->size *= 2;
}

extern void insert_value_to_pqueue(Pqueue_ptr pqueue, Collisions_list_elem *map_elem_ptr)
{
    ++pqueue->curr_size;
    if (pqueue->curr_size > pqueue->size)
    {
        reallocate_pqueue(pqueue);
    }

    pqueue->data[pqueue->curr_size - 1].map_elem_ptr = map_elem_ptr;
    pqueue->data[pqueue->curr_size - 1].last_using_time = time(NULL);

    shift_up(pqueue, pqueue->curr_size - 1);
}

/**
 * \brief Extract minimal by last_using_time element
 * \return Pointer to this element in hash_map
 */
extern Collisions_list_elem *extract_min(Pqueue_ptr pqueue)
{
    if (pqueue->curr_size == 0)
    {
        return NULL;
    }
    Collisions_list_elem *min = pqueue->data[0].map_elem_ptr;

    pqueue->data[0] = pqueue->data[pqueue->curr_size - 1];
    --pqueue->curr_size;
    shift_down(pqueue, 0);

    return min;
}

/**
 * \brief Delete priority queue element by block pointer
 */
extern void delete_from_pqueue(Pqueue_ptr pqueue, void *block_ptr)
{
    if (pqueue->curr_size == 0)
    {
        return;
    }

    int index = get_pqueue_element(pqueue, block_ptr);
    if (index < 0)
    {
        return;
    }

    pqueue->data[index] = pqueue->data[pqueue->curr_size - 1];
    --pqueue->curr_size;
    shift_down(pqueue, index);
}

/**
 * \brief Get priority queue element by block pointer
 * \return Index of element or NULL if element was not found
 */
extern int get_pqueue_element(Pqueue_ptr pqueue, void *block_ptr)
{
    for (size_t i = 0; i < pqueue->curr_size; i++)
    {
        if (pqueue->data[i].map_elem_ptr->block_ptr == block_ptr)
        {
            return (int) i;
        }
    }

    return -1;
}

/**
 * \brief Update last element using time
 */
extern void update_element_time(Pqueue_ptr pqueue, void *block_ptr)
{
    int index = get_pqueue_element(pqueue, block_ptr);
    pqueue->data[index].last_using_time = time(NULL);
    shift_down(pqueue, index);
}

extern void destroy_pqueue(Pqueue_ptr pqueue)
{   
    free(pqueue->data);
    free(pqueue);
}