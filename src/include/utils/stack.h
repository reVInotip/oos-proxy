#pragma once

#include <stdlib.h>

typedef struct stack_elem
{
    void *lib_handle;
    struct stack_elem *next_elem;   
} Stack_elem;

typedef Stack_elem *Stack_ptr;

extern Stack_ptr create_stack();
extern void push_to_stack(Stack_ptr *stack, void *element);
extern void* stack_top(Stack_ptr stack);
extern void destroy_stack(Stack_ptr *stack);
extern size_t get_stack_size(Stack_ptr stack);