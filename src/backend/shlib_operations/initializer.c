#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "utils/stack.h"
#include "shlib_operations/operations.h"
#include "logger/logger.h"
#include "boss_operations.h"
#include "master.h"

/**
    \brief Initialize all extensions (call init function with some args)
    \param [in] args - arguments for extension init function
    \return nothing
*/
void init_all_exetensions()
{   
    assert(lib_stack != NULL);

    Stack_ptr curr_stack = lib_stack;

    for (size_t i = 0; i < get_stack_size(lib_stack); i++)
    {
        void (*function)() = dlsym(curr_stack->data, "init");
        if (function == NULL)
        {
            elog(WARN, "%s\n", dlerror());
            curr_stack = curr_stack->next_elem;
            continue;
        }
        function();
        exec_requests(curr_stack->data);

        curr_stack = curr_stack->next_elem;
    }

    if (executor_start_hook) {
        executor_start_hook();
    }
}