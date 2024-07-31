#include "../../include/memory/memcache_map.h"
#include "../../include/boss_operations/boss_operations.h"
#include "../../include/memory/cache.h"
#include "../../include/logger/logger.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

Op_stack_ptr boss_op;

// ==========auxiliary structure for storing operations(stack)=============

static inline Op_stack_ptr create_stack()
{
    return NULL;
}

static void push_to_stack(Op_stack_ptr *stack, Boss_op_name op_name, void *operation_data)
{   
    Op_stack_elem *new_element = (Op_stack_elem *) malloc(sizeof(Op_stack_elem));
    assert(new_element != NULL);
    
    new_element->op_name = op_name;
    new_element->operation_data = operation_data;
    new_element->next_elem = *stack;
    *stack = new_element;
}

static void* pop_from_stack(Op_stack_ptr *stack, Boss_op_name *op_name)
{   
    *op_name = (*stack)->op_name;
    void *operation_data = (*stack)->operation_data;

    Op_stack_elem *elem_for_erase = *stack;
    *stack = (*stack)->next_elem;
    free(elem_for_erase);
    return operation_data;
}

//============operations for shared libraries=============

extern void init_boss_op()
{
    boss_op = create_stack();
}

extern int cache_write_op
(
    const char *key, 
    const char *message,
    const size_t key_length,
    const size_t message_length,
    const unsigned TTL
)
{
    if (key_length > MAX_KEY_SIZE)
    {
        return -1;
    }

    // when the operation execution time is added,
    // it will be possible to make a delayed write to the cache

    return cache_write(key, message, message_length, TTL);
}

extern void print_cache_op(const char *key, const size_t key_length)
{
    if (key_length > MAX_KEY_SIZE)
    {
        return;
    }

    char *message = (char *) cache_read(key, NULL, 0);
    printf("%s\n", message);
}

extern void register_background_worker(char *callback_name, char *bg_worker_name)
{
    if (callback_name == NULL || bg_worker_name == NULL)
    {
        elog(WARN, "Invalid arguments");
        return;
    }

    BGWorker_data *bg_worker_data = (BGWorker_data *) malloc(sizeof(BGWorker_data));
    assert(bg_worker_data);

    bg_worker_data->callback_name = (char *) malloc(strlen(callback_name) + 1);
    strcpy(bg_worker_data->callback_name, callback_name);

    bg_worker_data->bg_worker_name = (char *) malloc(strlen(bg_worker_name) + 1);
    strcpy(bg_worker_data->bg_worker_name, bg_worker_name);

    push_to_stack(&boss_op, REGISTER_BG_WORKER, bg_worker_data);
}

//============operations for boss=============

extern inline void clear_bg_worker_info(BGWorker_data *bg_worker_data)
{
    free(bg_worker_data->bg_worker_name);
    free(bg_worker_data->callback_name);
    free(bg_worker_data);
}

extern inline void *get_stack_top(Boss_op_name *op_name)
{
    if (boss_op == NULL)
    {
        return NULL;
    }
    return pop_from_stack(&boss_op, op_name);
}
