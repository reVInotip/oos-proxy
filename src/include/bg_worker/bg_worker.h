/**
 * bg_worker.h
 * This file contains structure describing background worker and function for work with it.
 */
#include "../boss_operations/boss_operations.h"

#pragma once

/**
 * \brief This struct describing backgound worker
 * \details It conrains name for identify it in logs, pointer to stack
 * pointer to the stack to clear it correctly and pid to complete the process correctly
 */
typedef struct bgworker
{
    char  *name;
    void *stack;
    int     pid;
} BGWorker;

extern void create_bg_worker(void *extension_handle, const BGWorker_data *bg_worker_data);
extern void drop_all_workers();