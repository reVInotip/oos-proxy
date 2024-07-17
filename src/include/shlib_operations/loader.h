#pragma once

#include "../utils/stack.h"

/*
descr: Find and load all shared libraties
args: stack - stack of shared libraries, path_to_source - path to the directory containing extensions,
*/
#define loader(stack, path_to_source) \
    do \
    {\
        default_loader(stack, path_to_source, 0, 1); \
    } while (0)

extern void default_loader(Lib_stack_ptr *stack, char *path_to_source, int curr_depth, const int depth);