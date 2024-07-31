/**
 * boss_operations.h
 * This file contains declarations of functions that can be called in a shared library init method
 * and functions by which the main process can perform the operations provided by those functions.
 */
#include <stdlib.h>

#pragma once

/**
 * \brief This is a names of this operations.
 */
typedef enum boss_op_name
{
    GET_SHMEM, // Ask main process (Boss) allocate shared memory
    REGISTER_BG_WORKER, // Ask the main process to register new background worker
    WRITE_TO_CACHE
} Boss_op_name;

/*typedef enum boss_op_time
{
    AFTER_START,
    AFTER_INIT,
    NOW
} Boss_op_time;*/

/**
 * \brief This struct describe operations which will be store in operations stack
 */
typedef struct opearations_stack_elem
{   
    Boss_op_name                     op_name;
    //Boss_op_time                   op_time;
    void                     *operation_data; // pointer to struct with data for operation
    struct opearations_stack_elem *next_elem;
} Op_stack_elem;

typedef Op_stack_elem *Op_stack_ptr;

/**
 * \brief Data for operations with background worker
 * \details callback_name - the name of the function with which the new process will start executing
 * bg_worker_name - name of new background worker
 */
typedef struct bg_worker_data
{
    char *callback_name;
    char *bg_worker_name;
} BGWorker_data;

/**
 * \brief This struct contians all operations functions. It is passed as the first argument to
 * the function init
 */
typedef struct boss_op_func
{  
    int (*cache_write_op)  (const char *key,
                            const char *mesasge,
                            const size_t key_length,
                            const size_t message_length,
                            const unsigned TTL);
    void (*print_cache_op) (const char *key, const size_t key_length);
    void (*register_background_worker) (char *callback_name, char *bg_worker_name);
} Boss_op_func;

extern void init_boss_op();
extern int cache_write_op
(
    const char *key, 
    const char *mesasge,
    const size_t key_length,
    const size_t message_length,
    const unsigned TTL
);
extern void print_cache_op(const char *key, const size_t key_length);
extern void register_background_worker(char *callback_name, char *bg_worker_name);

extern void *get_stack_top(Boss_op_name *op_name);
extern void clear_bg_worker_info(BGWorker_data *bg_worker_data);