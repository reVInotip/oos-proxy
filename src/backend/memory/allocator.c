#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include "../../include/logger/logger.h"
#include "../../include/guc/guc.h"
#include "../../include/memory/allocator.h"
#include "../../include/memory/cache_errno.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern enum cache_err_num cache_errno;

bool allocator_inited = false;

// limit on the block size that we can allocate in RAM
// if it is exceeded then the cache file is used
#define MAX_RAM_BLOCK_SIZE 1024 //1 Kb;

// the number of extra bytes that can be allocated for a block in RAM
#define ALLOWABLE_OFFSET 2

// pointer to start of user availiable memory section
void *start_ptr;

// pointer to all allocated memory
void *main_ptr;

// check if block is pointer to cache file
#define is_file(bool_field) ((bool_field & 0b10000000) >= 1)

// check if block is free
#define is_free(bool_field) ((bool_field & 0b01000000) >= 1)

// set that block is file pointer
#define set_file(bool_field) \
    do { \
        bool_field = bool_field | 0b10000000; \
    } while (0)

// set that block is free
#define set_free(bool_field) \
    do { \
        bool_field = bool_field | 0b01000000; \
    } while (0)

// set that block is not free
#define set_not_free(bool_field) \
    do { \
        bool_field = bool_field & 0b10111111; \
    } while (0)

// set that block is not file
#define set_not_file(bool_field) \
    do { \
        bool_field = bool_field & 0b01111111; \
    } while (0)

/**
 * \brief Maximum binary heap of free blocks
 * \details I maximum binary heap to save pointers on free blocks of memory.
 * This is necessary to quickly search, change and delete free blocks. The queue is stored before the block accessible
 * to the user (start_ptr points to it).(((size_t) max_allocation_size.num) / (sizeof(Block_header) + 1)) / 2 + 1
 */
typedef struct heap_elem
{   
    int64_t        free_block_size; // size is used as key fro heap
    Block_header *block_header_ptr;
} Heap_elem;

/**
 * \brief Meta data of heap
 * \details curr_size - curr_size of heap
 *          size - max size of heap. It`s calculated by the formula:
 * MAX_POSSIBLE_COUNT_OF_BLOCKS * SIZE_OF_THIS_STRUCT. First constant calculated by the formula:
 * (MAX_ALLOCATION_SIZE / (SIZE_OF_THIS_STRUCT + 1)) / 2 + 1. The idea is that the maximum possible number
 * of blocks is obtained by interleaving blocks of the minimum size. Therefore, we cut the resulting quantity in half.
 * Minimus size of block is SIZE_OF_THIS_STRUCT + 1. One is added to round the resulting value up.
 */
typedef struct heap_meta_data
{
    int64_t curr_size;
    int64_t      size;
} Heap_meta_data;

/**
 * \brief Swap two element of heap
 */
static void swap(Heap_elem *heap, int i, int j)
{
    // Also take into account the rearrangement in the structure of the blocks themselves
    heap[i].block_header_ptr->block_index = j;
    heap[j].block_header_ptr->block_index = i;

    Heap_elem tmp = heap[i];
    heap[i] = heap[j];
    heap[j] = tmp;
}

/**
 * \brief Get a pointer to a heap from a pointer to allocated memory
 */
static inline Heap_elem *get_heap()
{
    return (Heap_elem *) ((char *) main_ptr + sizeof(Heap_meta_data));
}

/**
 * \brief Get a pointer to a heap meta data from a pointer to allocated memory
 */
static inline Heap_meta_data *get_heap_meta_data()
{
    return (Heap_meta_data *) main_ptr;
}

// ===== heap balancing procedures =====

/**
 * \brief If the value of an element has increased, then we move it up the heap in this function
 * \param [in] i - index if this element
 * \return New index of this element
 */
static int shift_up(int i)
{
    Heap_elem *heap = get_heap();
    while (heap[i].free_block_size > heap[(i - 1) / 2].free_block_size)
    {
        swap(heap, i, (i - 1) / 2);
        i = (i - 1) / 2;
    }

    return i;
}

/**
 * \brief If the value of an element has decreased, then we move it down the heap in this function
 * \param [in] i - index if this element
 * \return New index of this element
 */
static int shift_down(int i)
{   
    Heap_elem *heap = get_heap();
    Heap_meta_data *meta_data = get_heap_meta_data();

    int left = 0;
    int right = 0;
    int j = 0;
    while (2 * i + 1 < meta_data->curr_size)
    {
        left = 2 * i + 1;
        right = 2 * i + 2;
        j = left;
        if (right < meta_data->curr_size &&
            heap[right].free_block_size > heap[left].free_block_size)
        {
            j = right;
        }

        if (heap[i].free_block_size >= heap[j].free_block_size)
        {
            break;
        }

        swap(heap, i, j);
        i = j;
    }

    return i;
}

/**
 * \brief Insert new free block to heap
 * \details We add this block to the end of the heap and move it up the heap as long as we can
 * \param [in] free_block_header_ptr - pointer to the block to be added
 * \param [in] block_size - size of this block (key for this heap)
 * \return Index in heap of this block
 */
static int insert_value_to_heap(Block_header *free_block_header_ptr, size_t block_size)
{
    Heap_elem *heap = get_heap();
    Heap_meta_data *meta_data = get_heap_meta_data();

    if (meta_data->curr_size >= meta_data->size)
    {
        return 0;
    }
    ++meta_data->curr_size;

    heap[meta_data->curr_size - 1].block_header_ptr = free_block_header_ptr;
    heap[meta_data->curr_size - 1].free_block_size = block_size;

    return shift_up(meta_data->curr_size - 1);
}


/**
 * \brief Delete free block from heap
 * \details We replace the block being removed with a block from the end and
 * move it down the heap as long as we can
 * \param [in] i - block index to delete
 */
static void delete_value_from_heap(int i)
{
    Heap_elem *heap = get_heap();
    Heap_meta_data *meta_data = get_heap_meta_data();

    if (meta_data->curr_size == 0)
    {
        return;
    }

    heap[i] = heap[meta_data->curr_size - 1];
    --meta_data->curr_size;
    shift_down(i);
}

/**
 * \brief Linear search for a free block of size >= then block_size (param) on the heap
 * \note I was thinking about sorting the heap (like heap sort) and then searching
 * for the element using binary search, but it seems like a simple linear search
 * would be faster. I also thought about using red-black trees for logarithmic element
 * searching, but decided that I didn't have enough time to do it.
 * \param [in] block_size - the size of the block we need
 * \return Block index of the most suitable size
 */
static int get_heap_element(const size_t block_size)
{
    Heap_elem *heap = get_heap();
    Heap_meta_data *meta_data = get_heap_meta_data();
    int last_preferred_element = 0;

    for (size_t i = 0; i < meta_data->size; i++)
    {
        if (heap[i].free_block_size == block_size)
        {
            return i;
        }
        else if (heap[i].free_block_size > block_size)
        {
            last_preferred_element = i;
        }
    }

    return last_preferred_element;
}
    
/**
 * \brief Quickly check whether memory of a given size can be
 * allocated (possible due to the properties of the maximum binary heap)
 * \param [in] block_size - size of memory we want to allocate
 * \return Can we allocate this memory?
 */
bool check_is_mem_availiable(const size_t block_size)
{
    Heap_elem *heap = get_heap();
    Heap_meta_data *meta_data = get_heap_meta_data();

    if (meta_data->size == 0)
    {
        return false;
    }

    return heap[0].free_block_size >= block_size;
}

/**
 * \brief Map dimension of data in varibale memory_for_cache to value
 * \return value if OK, else return -1
 */
static long unit_to_value(char *unit)
{
    if (strcmp(unit, "Gb") == 0)
        return 1024 * 1024 * 1024;
    else if (strcmp(unit, "Mb") == 0)
        return 1024 * 1024;
    else if (strcmp(unit, "Kb") == 0)
        return 1024;
    else if (strcmp(unit, "b") == 0)
        return 1;
    
    return -1;
}

/**
 * \brief Initialize allocator
 * \details mmap new region of size MAX_ALLOCATION_SIZ (sets in config) +
 * META_INFO_SIZE (size of heap and it meta data)
 * \return -1 if something went wrong, 0 if all is OK
 */
int init_OOS_allocator()
{
    if (!is_var_exists_in_config("memory_for_cache", C_MAIN))
        define_custom_long_variable("memory_for_cache", "Size of memory for cache", 5, C_MAIN | C_STATIC); // 5Mb
    
    if (!is_var_exists_in_config("unit_memory", C_MAIN))
        define_custom_string_variable("unit_memory", "Dimension of data in memory_for_cache variable", "Mb", C_MAIN | C_STATIC);
    

    Guc_data max_allocation_size = get_config_parameter("memory_for_cache", C_MAIN);
    Guc_data unit_str = get_config_parameter("unit_memory", C_MAIN);

    long unit = unit_to_value(unit_str.str);
    if (unit < 0)
    {
        elog(WARN, "Incorrect value in unit_memory variable: %s. Use default (Mb)", unit_str.str);
        unit = 1024 * 1024; // Mb
    }

    max_allocation_size.num *= unit;

    size_t max_count_of_free_blocks = (((size_t) max_allocation_size.num) / (sizeof(Block_header) + 1)) / 2 + 1;

    main_ptr = mmap(NULL, max_allocation_size.num + max_count_of_free_blocks * sizeof(Heap_elem) + sizeof(Heap_meta_data),
                                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (main_ptr == MAP_FAILED)
    {
        elog(ERROR, "Error while allocating memory: %s, errno: %d", strerror(errno), errno);
        return -1;
    }
    // initialize heap meta data
    Heap_meta_data *meta_data = get_heap_meta_data();
    meta_data->curr_size = 0;
    meta_data->size = max_count_of_free_blocks;

    start_ptr = (char *) main_ptr + max_count_of_free_blocks * sizeof(Heap_elem) + sizeof(Heap_meta_data);

    // by default we have only one large block which will be splited if necessary
    Block_header *main_header_ptr = (Block_header *) start_ptr;
    main_header_ptr->block_size = max_allocation_size.num - sizeof(Block_header);
    main_header_ptr->bool_constants = 0;
    main_header_ptr->next_block = NULL;
    main_header_ptr->prev_block = NULL;
    main_header_ptr->block_index = 0;
    set_free(main_header_ptr->bool_constants);

    main_header_ptr->block_index = insert_value_to_heap(main_header_ptr, main_header_ptr->block_size);

    allocator_inited = true;

    return 0;
}

/**
 * \details Split block if it necessary. If we can`t split block we return pointer to block given to us.
 * If we can we return pointer to new block. In both cases the returned block will be marked as not free
 * \param [in] block_header - header to block for split
 * \param [in] i - index of this block in heap
 * \param [in] size - required block size
 * \return Pointer to allocated block
 */
void *split_block_if_it_possible(Block_header *block_header, int i, const size_t size)
{
    // Check if we can split this block. The size of the original block must be no less than the size required of
    // the block + the size of the header for the new block + the minimum size that must remain for the old
    // block (ALLOWABLE_OFFSET). The correctness of the inequality without taking into account the constant (ALLOWABLE_OFFSET)
    // is guaranteed..
    if (block_header->block_size < size + ALLOWABLE_OFFSET + sizeof(Block_header))
    {
        delete_value_from_heap(i);
        set_not_free(block_header->bool_constants);
        return (char *) block_header + sizeof(Block_header);
    }

    // we add the size of block header to shift the pointer and subtract it to get the length of the new block
    Block_header *new_block = (Block_header *) ((char *) block_header + block_header->block_size - size);
    new_block->block_size = size;
    new_block->bool_constants = 0;
    new_block->next_block = block_header->next_block;
    new_block->prev_block = block_header;

    if (new_block->next_block != NULL)
    {
        new_block->next_block->prev_block = new_block;
    }

    block_header->next_block = new_block;
    block_header->block_size = block_header->block_size - size - sizeof(Block_header);

    Heap_elem *heap = get_heap();
    heap[i].free_block_size = block_header->block_size - sizeof(Block_header);

    block_header->block_index = shift_down(i);

    return (char *) new_block + sizeof(Block_header);
}

/**
 * \brief Allocate memory
 * \param [in] count_blocks - count blocks for allocate
 * \note if count_blocks == 1 allocate block else allocate chain
 * \return NULL if there is not enough space in the cache for a block of this size,
 * pointer to start block if all is OK. If cache runs out of free space or block size to large
 * then cache_errno enum variable will receive the corresponding code. This is a signal that it's
 * time to call LRU.
 */
void *OOS_allocate(const size_t size)
{
    print_alloc_mem();
    Heap_elem *heap = get_heap();
    if (!check_is_mem_availiable(size))
    {
        elog(ERROR, "No free space in cache");
        cache_errno = NO_FREE_SPACE;
        return NULL;
    }

    int index = get_heap_element(size + sizeof(Block_header));
    void *free_block = split_block_if_it_possible(heap[index].block_header_ptr, index, size);

    return free_block;
}

/**
 * \brief Free block by address
 * \param [in] addr - address of block we should free
 */
void OOS_free(const void *addr)
{
    Block_header *block = (Block_header *) ((char *) addr - sizeof(Block_header));
    set_free(block->bool_constants);

    // join next block with this block if it possible
    if (block->next_block != NULL && is_free(block->next_block->bool_constants))
    {
        delete_value_from_heap(block->next_block->block_index);
        if (block->next_block->next_block != NULL)
        {
            block->next_block->next_block->prev_block = block;
        }

        block->block_size += block->next_block->block_size + sizeof(Block_header);
        block->next_block = block->next_block->next_block;
    }

    // join previous block with this block if it possible
    if (block->prev_block != NULL && is_free(block->prev_block->bool_constants))
    {   
        if (block->next_block != NULL)
        {
            block->next_block->prev_block = block->prev_block;
        }

        block->prev_block->block_size += block->block_size + sizeof(Block_header);
        block->prev_block->next_block = block->next_block;

        Heap_elem *heap = get_heap();
        heap[block->prev_block->block_index].free_block_size = block->prev_block->block_size;

        shift_up(block->prev_block->block_index);
    }
    // if we can`t join this block we insert to heap
    else
    {
        block->block_index = insert_value_to_heap(block, block->block_size);
    }
}

/**
 * \brief Destroy allocator and free all regions that it allocate
 */
void destroy_OOS_allocator()
{
    if (!allocator_inited)
        return;
        
    Guc_data max_allocation_size = get_config_parameter("memory_for_cache", C_MAIN);
    Guc_data unit_str = get_config_parameter("unit_memory", C_MAIN);

    long unit = unit_to_value(unit_str.str);
    if (unit < 0)
        unit = 1024 * 1024; // Mb

    max_allocation_size.num *= unit;

    Heap_meta_data *meta_data = get_heap_meta_data();

    munmap(main_ptr, max_allocation_size.num + meta_data->size * sizeof(Heap_elem) + sizeof(Heap_meta_data));
}

void print_alloc_mem()
{   
    Block_header *curr_block = (Block_header *) start_ptr;
    printf("Block index | Block address | Block size | Is free | Is file | Next header element | Prev header element\n");
    while (curr_block != NULL)
    {
        printf("Block %u | %p | %d | %d | %d | %p | %p\n", curr_block->block_index, (char *) curr_block, curr_block->block_size,
                is_free(curr_block->bool_constants), is_file(curr_block->bool_constants), curr_block->next_block, curr_block->prev_block);
        curr_block = curr_block->next_block;
    }
}
