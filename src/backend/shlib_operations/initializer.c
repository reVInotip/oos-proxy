#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "utils/stack.h"
#include "shlib_operations/operations.h"
#include "logger/logger.h"
#include "boss_operations/boss_operations.h"
#include "boss_operations/hook.h"

/**
    \brief Initialize all extensions (call init function with some args)
    \param [in] lib_stack - stack which contains shared libraries handles
    \param [in] args - arguments for extension init function
    \return nothing
*/
void init_all_exetensions(Stack_ptr lib_stack)
{   
    assert(lib_stack != NULL);

    Stack_ptr curr_stack = lib_stack;

    Boss_op_func *op_func = malloc(sizeof(Boss_op_func));
    op_func->cache_write_op = cache_write_op;
    op_func->print_cache_op = print_cache_op;
    op_func->register_background_worker = register_background_worker;
    op_func->define_custom_long_variable_op = define_custom_long_variable_op;
    op_func->define_custom_string_variable_op = define_custom_string_variable_op;

    for (size_t i = 0; i < get_stack_size(lib_stack); i++)
    {
        void (*function)() = dlsym(curr_stack->data, "init");
        if (function == NULL)
        {
            elog(WARN, "%s\n", dlerror());
            curr_stack = curr_stack->next_elem;
            continue;
        }
        function(op_func);
        exec_requests(curr_stack->data);

        curr_stack = curr_stack->next_elem;
    }
    free(op_func);

    if (executor_start_hook) {
        executor_start_hook();
    }
}