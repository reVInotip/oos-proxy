#define _POSIX_C_SOURCE 199310

#include "logger/logger.h"
#include "boss_operations/boss_operations.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

void sigint_handler(int num)
{
    _exit(5);
}

void init(void *arg, ...)
{
    Boss_op_func *func = (Boss_op_func *) arg;
    func->register_background_worker("my_bgworker", "my_bgworker", false);
}

void my_bgworker()
{
    struct sigaction sa = { .__sigaction_handler = { sigint_handler } };
    sigaction(SIGTERM, &sa, NULL);

    while (1)
    {
        elog(LOG, "log_healthcheck");
        sleep(1);
    }
}