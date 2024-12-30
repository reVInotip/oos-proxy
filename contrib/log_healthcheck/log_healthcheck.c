#include "logger/logger.h"
#include "bg_worker/bg_worker.h"
#include "master.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>


extern void init()
{
    register_background_worker("my_bgworker", "log_healthcheck", false);
}

void my_bgworker()
{
    while (1)
    {
        elog(LOG, "log_healthcheck");
        sleep(1);
    }
}