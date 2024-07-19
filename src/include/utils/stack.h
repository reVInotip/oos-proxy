#pragma once

#include <stdlib.h>

typedef struct stack_elem
{   
    void *data;
    struct stack_elem *next_elem;   
} Stack_elem;

typedef Stack_elem *Stack_ptr;

extern Stack_ptr create_stack();
extern void push_to_stack(Stack_ptr *stack, void *element);
extern void *stack_top(Stack_ptr stack);
extern void destroy_stack(Stack_ptr *stack);
extern void *get_stack_element(Stack_ptr stack, size_t index);
extern size_t get_stack_size(Stack_ptr stack);