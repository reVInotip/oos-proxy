#define _POSIX_C_SOURCE 199310

#include "logger/logger.h"
#include "boss_operations/boss_operations.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

void init(void *arg, ...)
{
    Boss_op_func *func = (Boss_op_func *) arg;
    func->register_background_worker("сache_keeper", "cache_keeper", false);
}

/**
 * \brief Clean cache by deleting blocks with ELAPSED TTL
 * \details This function should be execute in separate process or thread.
 *  Cache cleaning runs every 3 seconds.
 */
void cache_keeper()
{
}