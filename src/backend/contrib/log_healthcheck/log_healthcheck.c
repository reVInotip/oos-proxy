#include "../../../include/logger/logger.h"
#include "../../../include/boss_operations/boss_operations.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

extern void init(void *arg, ...)
{
    Boss_op_func *func = (Boss_op_func *) arg;
    func->register_background_worker("my_bgworker", "my_bgworker", true);
}

void my_bgworker()
{
    while (1)
    {
        elog(INFO, "log_healthcheck");
        sleep(1);
    }
}