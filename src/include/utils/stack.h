/**
 * stack.h
 * Structure describing the stack and methods for it.
 * I use it to store handles of shared libraries.
 */
#include <stdlib.h>

#pragma once

typedef struct stack_elem
{   
    void *data;
    struct stack_elem *next_elem;  
} Stack_elem;

typedef Stack_elem *Stack_ptr;

extern Stack_ptr create_stack();
extern void push_to_stack(Stack_ptr *stack, void *element);
extern void* pop_from_stack(Stack_ptr *stack);
extern void *stack_top(Stack_ptr stack);
extern void destroy_stack(Stack_ptr *stack);
extern size_t get_stack_size(Stack_ptr stack);