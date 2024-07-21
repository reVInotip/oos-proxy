/**
 * operations.h
 * This file contains basic operations on shared libraries such as
 * load to shared memory, removal from it and initialization.
 * 
 */

#pragma once

#include "../utils/stack.h"

/**
 * \brief Find and load all shared libraties
 * \param [in] stack - stack in which libraries will be added
 * \param [in] path_to_source - directory which contains shared libraries (.so files)
 * \return nothing
 */
#define loader(stack, path_to_source) \
    do \
    {\
        default_loader(stack, path_to_source, 0, 1); \
    } while (0)

extern void default_loader(Stack_ptr *stack, char *path_to_source, int curr_depth, const int depth);
extern void close_all_exetensions(Stack_ptr lib_stack);
extern void init_all_exetensions(Stack_ptr lib_stack);