#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "../../include/utils/stack.h"

extern inline Stack_ptr create_stack()
{
    return NULL;
}

extern void* stack_top(Stack_ptr stack)
{
    assert(stack != NULL);

    return stack->lib_handle;
}

extern void push_to_stack(Stack_ptr *stack, void *element)
{   
    Stack_elem *new_element = (Stack_elem *) malloc(sizeof(Stack_elem));
    assert(new_element != NULL);
    
    new_element->lib_handle = element;
    new_element->next_elem = *stack;
    *stack = new_element;
}

extern void* pop_from_stack(Stack_ptr *stack)
{
    assert(stack != NULL);

    void *elem_data = (*stack)->lib_handle;
    Stack_elem *elem_for_erase = *stack;
    *stack = (*stack)->next_elem;
    free(elem_for_erase);
    return elem_data;
}

extern void destroy_stack(Stack_ptr *stack)
{
    assert(stack != NULL);

    Stack_ptr curr_stack = NULL;
    while (stack != NULL)
    {
        curr_stack = *stack;
        *stack = (*stack)->next_elem;
        free(curr_stack);
    }
}

extern size_t get_stack_size(Stack_ptr stack)
{
    size_t size = 0;

    Stack_ptr curr_stack = stack;
    while (curr_stack != NULL)
    {
        ++size;
        curr_stack = curr_stack->next_elem;
    }

    return size;
}