#pragma once

typedef struct lib_stack_elem
{
    void *lib_handle;
    struct lib_stack_elem *next_elem;   
} Lib_stack_elem;

typedef Lib_stack_elem *Lib_stack_ptr;

extern Lib_stack_ptr create_stack();
extern void push(Lib_stack_ptr *stack, void *element);
extern void* pop(Lib_stack_ptr *stack);
extern void* top(Lib_stack_ptr stack);
extern void destroy_stack(Lib_stack_ptr *stack);
extern size_t get_size(Lib_stack_ptr stack);