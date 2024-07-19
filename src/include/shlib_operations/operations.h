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


/**
 * \brief Find and load all shared libraties
 * \param [in] stack - stack in which libraries will be added
 * \param [in] path_to_source - directory which contains shared libraries (.so files)
 * \param [in] curr_depth - current scan depth relative to the start directory (0 by default; this parameter need for recursion)
 * \param [in] depth - max scan depth relative to the start directory
 * \return nothing
 */
extern void default_loader(Stack_ptr *stack, char *path_to_source, int curr_depth, const int depth);

/**
    \brief Close all extensions
    \param [in] lib_stack - stack which contains shared libraries handles
    \return nothing
*/
extern void close_all_exetensions(Stack_ptr lib_stack);

/**
    \brief Initialize all extensions
    \param [in] lib_stack - stack which contains shared libraries handles
    \param [in] args - arguments for extension init function
    \return nothing
*/
extern void init_all_exetensions(Stack_ptr lib_stack);