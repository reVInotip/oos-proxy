#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils/stack.h"
#include "logger/logger.h"
#include "shlib_operations/operations.h"
#include "master.h"

/**
    \brief Close all extensions and unload it from memory
    \return nothing
*/
void close_all_exetensions()
{   
    if (lib_stack == NULL)
    {
        return;
    }

    if (executor_end_hook) {
        executor_end_hook();
    }

    Stack_ptr curr_stack = lib_stack;
    for (size_t i = 0; i < get_stack_size(lib_stack); i++)
    {
        if (dlclose(curr_stack->data))
        {
            elog(ERROR, "Can`t close extension");
        }

        curr_stack = curr_stack->next_elem;
    }
}