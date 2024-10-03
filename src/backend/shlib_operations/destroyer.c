#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../include/utils/stack.h"
#include "../../include/logger/logger.h"
#include "../../include/shlib_operations/operations.h"

/**
    \brief Close all extensions and unload it from memory
    \param [in] lib_stack - stack which contains shared libraries handles
    \return nothing
*/
extern void close_all_exetensions(Stack_ptr lib_stack)
{   
    if (lib_stack == NULL)
    {
        return;
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