#define _GNU_SOURCE

#include <stdio.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include "../../include/boss_operations.h"
#include "../../include/shlib_operations/operations.h"
#include "../../include/logger/logger.h"
#include "../../include/bg_worker/bg_worker.h"

extern void exec_requests(void *extension_handle)
{
    void *operation_data;
    Boss_op_name op_name;
    while ((operation_data = get_stack_top(&op_name)) != NULL)
    {
        switch (op_name)
        {
            case REGISTER_BG_WORKER:
                BGWorker_data * bg_worker_data = (BGWorker_data *) operation_data;
                if (bg_worker_data->need_observer)
                {
                    create_bg_worker_tracer(extension_handle, bg_worker_data);
                }
                else
                {
                    create_bg_worker(extension_handle, bg_worker_data);
                }
                clear_bg_worker_info(bg_worker_data);
                break;
            
            default:
                break;
        }
    }
}