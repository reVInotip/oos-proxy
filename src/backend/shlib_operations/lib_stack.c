#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "../../include/shlib_operations/lib_stack.h"

extern inline Lib_stack_ptr create_stack()
{
    return NULL;
}

extern void* top(Lib_stack_ptr stack)
{
    assert(stack != NULL);

    return stack->lib_handle;
}

extern void push(Lib_stack_ptr *stack, void *element)
{   
    Lib_stack_elem *new_element = (Lib_stack_elem *) malloc(sizeof(Lib_stack_elem));
    assert(new_element != NULL);
    
    new_element->lib_handle = element;
    new_element->next_elem = *stack;
    *stack = new_element;
}

extern void* pop(Lib_stack_ptr *stack)
{
    assert(stack != NULL);

    void *elem_data = (*stack)->lib_handle;
    Lib_stack_elem *elem_for_erase = *stack;
    *stack = (*stack)->next_elem;
    free(elem_for_erase);
    return elem_data;
}

extern void destroy_stack(Lib_stack_ptr *stack)
{
    assert(stack != NULL);

    Lib_stack_ptr curr_stack = NULL;
    while (stack != NULL)
    {
        curr_stack = *stack;
        *stack = (*stack)->next_elem;
        free(curr_stack);
    }
}

extern size_t get_size(Lib_stack_ptr stack)
{
    size_t size = 0;

    Lib_stack_ptr curr_stack = stack;
    while (curr_stack != NULL)
    {
        ++size;
        curr_stack = curr_stack->next_elem;
    }

    return size;
}