#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <libconfig.h>
#include <string.h>
#include <assert.h>
#include "../../include/utils/stack.h"
#include "../../include/shlib_operations/operations.h"

/**
    \brief Initialize all extensions (call init function with some args)
    \param [in] lib_stack - stack which contains shared libraries handles
    \param [in] args - arguments for extension init function
    \return nothing
*/
extern void init_all_exetensions(Stack_ptr lib_stack)
{   
    assert(lib_stack != NULL);

    Stack_ptr curr_stack = lib_stack;
    for (size_t i = 0; i < get_stack_size(lib_stack); i++)
    {
        void (*function)() = dlsym(curr_stack->data, "init");
        function();

        curr_stack = curr_stack->next_elem;
    }
}