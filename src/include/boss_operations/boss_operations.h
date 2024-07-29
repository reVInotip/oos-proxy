#include <stdlib.h>

#pragma once

typedef enum boss_op_name
{
    GET_SHMEM,
    REGISTER_BG_WORKER,
    WRITE_TO_CACHE
} Boss_op_name;

/*typedef enum boss_op_time
{
    AFTER_START,
    AFTER_INIT,
    NOW
} Boss_op_time;*/

typedef struct boss_operation
{
    Boss_op_name op_name;
    //Boss_op_time op_time;
    void * operation_data; // pointer to struct with data for operation
} Boss_operation;

typedef struct boss_op_func
{  
    int (*cache_write_op) (const char *key, const char *mesasge, const size_t key_length, const size_t message_length);
    void (*print_cache_op) (const char *key, const size_t key_length);
} Boss_op_func;

void init_boss_op();
int cache_write_op
(
    const char *key, 
    const char *mesasge,
    const size_t key_length,
    const size_t message_length
);
void print_cache_op(const char *key, const size_t key_length);