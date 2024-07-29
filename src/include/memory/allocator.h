/**
 * allocator.h
 * This file contains external methods and struct of block header for OOS allocator (aka heap).
 * Using this allocator you can allocate memory of arbitrary size.
 * The maximum memory size for allocation is set in the config. Memory for the maximum heap of
 * free blocks is also added to this size (more details in the description of the corresponding functions).
 * When memory is freed, free blocks are merged into one.
 * WARNING: The allocator does not have additional checks for the correctness of the address.
 *          Passing an incorrect address to a function = undefined behavior!
 * 
 * TO-DO add saving to file
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>

// if MAP_ANONYMOUS is undefined
#ifndef MAP_ANONYMOUS
    #define MAP_ANONYMOUS 0x20
#endif

/**
 * \brief This struct discribes header of block
 * \details next_block and prev_block pointers are used to combat memory fragmentation.
 *          bool_constaints used to save space on boolean values.
 */
typedef struct block_header
{
    struct block_header  *next_block;
    struct block_header  *prev_block;
    int32_t               block_size;
    uint16_t             block_index; // block index in heap
    int8_t            bool_constants;
} Block_header;

extern int init_OOS_allocator();
extern void *OOS_allocate(const size_t size);
extern void OOS_free(const void *addr);
extern void destroy_OOS_allocator();
extern void print_alloc_mem();
extern bool check_is_mem_availiable(const size_t block_size);