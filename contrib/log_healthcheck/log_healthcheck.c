#include "logger/logger.h"
#include "boss_operations/boss_operations.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

void sigint_handler() {
    exit(5);
}

void init(void *arg, ...)
{
    Boss_op_func *func = (Boss_op_func *) arg;
    func->register_background_worker("my_bgworker", "my_bgworker", false);
}

void my_bgworker()
{
    signal(SIGINT, sigint_handler);
    
    while (1)
    {
        elog(LOG, "log_healthcheck");
        sleep(1);
    }
}