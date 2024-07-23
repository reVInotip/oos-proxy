/**
 * allocator.h
 * This file contains external methods of OOS pool allocator.
 * Using this allocator you can allocate memory in fixed size blocks.
 * Their number is indicated in the config. You can also create chains of such blocks
 * using special methods. More details in the description of methods.
 * 
 * TO-DO add saving to file
 */

#pragma once

#include <stdlib.h>

//#define MAX_ALLOCATION_SIZE 8192
// Max size of one block in bytes
#define MAX_BLOCK_SIZE 128

// if MAP_ANONYMOUS is undefined
#ifndef MAP_ANONYMOUS
    #define MAP_ANONYMOUS 0x20
#endif

extern int init_OOS_allocator();
extern void *OOS_allocate_block();
extern int OOS_allocate_chain(void *block_addresses[], const size_t count_blocks, const size_t array_length);
extern int OOS_save_to_chain(const void *start_addr, const size_t length, const char *data);
extern void OOS_free_block(const void *block_addr);
extern void OOS_free_chain(const void *block_addr);
extern void destroy_OOS_allocator();
extern void print_OOS_alloc_mem();