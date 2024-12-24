#include "memory/memcache_map.h"
#include "boss_operations/boss_operations.h"
#include "memory/cache.h"
#include "logger/logger.h"
#include "guc/guc.h"
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

void init_boss_op()
{
    boss_op = create_stack();
}

int cache_write_op
(
    const char *key, 
    const char *message,
    const size_t message_length,
    const unsigned TTL
)
{
    if (strlen(key) + 1 > MAX_KEY_SIZE)
    {
        elog(ERROR, "Size of arguments too large");
        return -1;
    }

    // when the operation execution time is added,
    // it will be possible to make a delayed write to the cache

    return cache_write(key, message, message_length, TTL);
}

void print_cache_op(const char *key)
{
    if (strlen(key) + 1 > MAX_KEY_SIZE)
    {
        elog(ERROR, "Size of arguments too large");
        return;
    }
    
    Cache_data_t message = cache_read(key, NULL, 0);
    elog(LOG, "%s\n", (char *) message.ptr);
}

void register_background_worker(char *callback_name, char *bg_worker_name, bool need_observer)
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

    bg_worker_data->need_observer = need_observer;

    push_to_stack(&boss_op, REGISTER_BG_WORKER, bg_worker_data);
}

void define_custom_long_variable_op(char *name, const char *descr, const long boot_value, const int8_t context)
{
    if (strlen(name) + 1 > MAX_NAME_LENGTH || strlen(descr) + 1 > MAX_DESCRIPTION_LENGTH)
    {
        elog(ERROR, "Size of arguments too large");
        return;
    }

    if (is_main(context))
    {
        elog(ERROR, "Try to create main context variable from user context function");
        return;
    }

    define_custom_long_variable(name, descr, boot_value, context | C_USER);
}

void define_custom_string_variable_op(char *name, const char *descr, const char *boot_value, const int8_t context)
{
    if 
    (
        strlen(name) + 1 > MAX_NAME_LENGTH ||
        strlen(descr) + 1 > MAX_DESCRIPTION_LENGTH
    )
    {
        elog(ERROR, "Size of arguments too large");
        return;
    }

    if (is_main(context))
    {
        elog(ERROR, "Try to create main context variable from user context function");
        return;
    }

    define_custom_string_variable(name, descr, boot_value, context | C_USER);
}

char *get_config_string_parameter_op(const char *name, const int8_t context)
{
    if (strlen(name) + 1 > MAX_NAME_LENGTH)
    {
        elog(ERROR, "Size of arguments too large");
        return NULL;
    }

    Config_vartype vartype = get_config_parameter_type(name, context);
    if (vartype == UNINIT)
    {
        elog(ERROR, "Varibale with name %s not exists in this context\n", name);
        return NULL;
    } else if (vartype != STRING)
    {
        elog(ERROR, "Varibale with name %s isn`t string\n", name);
        return NULL;
    }

    Guc_data var = get_config_parameter(name, context);

    return var.str;
}

//============operations for boss=============

inline void clear_bg_worker_info(BGWorker_data *bg_worker_data)
{
    free(bg_worker_data->bg_worker_name);
    free(bg_worker_data->callback_name);
    free(bg_worker_data);
}

inline void *get_stack_top(Boss_op_name *op_name)
{
    if (boss_op == NULL)
    {
        return NULL;
    }
    return pop_from_stack(&boss_op, op_name);
}
