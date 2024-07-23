#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include "../../include/logger/logger.h"
#include "../../include/memory/allocator.h"
#include "../../include/guc/guc.h"

void *allocated_memory_ptr;
void *start_addr;

/**
 * \brief This structure describe element of header (user can not interact with it)
 * \details
 *      block_addr - virtual address of block start
 *      next_header_element - pointer to next header element of chain (NULL by default)
 *      is_free - is block free?
 *      is_file - if block contains pointer to FILE (false by default)
 */
typedef struct header_elem
{
    void *block_addr;
    struct header_elem *next_header_elem;
    bool is_free;
    bool is_file;
} Header_elem;

/**
 * \brief Find first free block address
 * \return Header of first free block
 */
Header_elem *get_free_block_addr()
{
    Header_elem *curr_ptr = (Header_elem *) allocated_memory_ptr;
    while (curr_ptr != start_addr)
    {
        if (curr_ptr->is_free)
        {
            return curr_ptr;
        }
        
        ++curr_ptr;
    }

    return NULL;
}

/**
 * \brief Get block header by its address
 * \param [in] block_addr - address of the block whose header
 *                          we want to receive
 * \return Header of block with address - block_addr
 */
Header_elem *get_block_header(const void *block_addr)
{
    Header_elem *curr_ptr = (Header_elem *) allocated_memory_ptr;
    while (curr_ptr != start_addr)
    {
        if (curr_ptr->block_addr == block_addr)
        {
            return curr_ptr;
        }
        
        ++curr_ptr;
    }

    return NULL;
}

/**
 * \brief Initialize allocator
 * \return -1 if something went wrong, 0 if all is OK
 */
extern int init_OOS_allocator()
{
    Guc_data max_allocation_size = get_config_parameter("memory_for_cache");

    size_t header_elem_size = sizeof(Header_elem);
    size_t count_blocks = max_allocation_size.num / MAX_BLOCK_SIZE;
    size_t header_size = count_blocks * header_elem_size;

    // allocate memory for header and pointers for blocks
    // this region will be private and anonymous (not mapped on file)
    // with access to read and write
    allocated_memory_ptr = mmap(NULL, sizeof(void *) * count_blocks + header_size,
                                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
                                -1, 0);
    if (allocated_memory_ptr == MAP_FAILED)
    {
        elog(ERROR, "Error while allocating memory: %s", strerror(errno));
        return -1;
    }

    // fill this memory
    Header_elem *curr_ptr = (Header_elem *) allocated_memory_ptr;
    for (size_t i = 0; i < count_blocks; i++)
    {
        // all blocks also will be private and anonymous
        // with access to read and write
        curr_ptr->block_addr = mmap(NULL, MAX_BLOCK_SIZE, PROT_READ | PROT_WRITE,
                                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (curr_ptr->block_addr == MAP_FAILED)
        {
            elog(ERROR, "Error while allocating memory: %s", strerror(errno));
            return -1;
        }                            
        curr_ptr->is_file = false;
        curr_ptr->is_free = true;
        curr_ptr->next_header_elem = NULL;

        ++curr_ptr;
    }
    

    start_addr = curr_ptr;

    return 0;  
}

/**
 * \brief Allocate only one block from pool
 * \return Pointer to start of this block
 */
extern void *OOS_allocate_block()
{
    Header_elem *header_of_free_block = get_free_block_addr();

    if (header_of_free_block == NULL)
    {
        elog(ERROR, "Heap buffer overflow");
    }

    header_of_free_block->is_free = false;

    return header_of_free_block->block_addr;
}

/**
 * \brief Allocate chain of blocks
 * \param [out] block_addresses - Addresses of blocks from chain
 * \param [in] count_blocks - count blocks for allocate
 * \param [in] array_length - length of array block_addresses
 * \return -1 if something went wrong, 0 if all is OK
 * \note Addresses are written to the array in order by block number in the chain
 */
extern int OOS_allocate_chain(void *block_addresses[], const size_t count_blocks, const size_t array_length)
{
    if (array_length > count_blocks)
    {
        return -1;
    }

    Header_elem *header_of_free_block = get_free_block_addr();
    if (header_of_free_block == NULL)
    {
        elog(ERROR, "Heap buffer overflow");
        return -1;
    }

    Header_elem *chain_start_block = header_of_free_block;
    Header_elem *prev_header_block = header_of_free_block;

    for (size_t i = 0; i < count_blocks - 1; i++)
    {
        if (header_of_free_block == NULL)
        {
            // if we can not allocate another block
            // we should mark all alredy allocatied blocks like free
            Header_elem *curr_block;
            while (chain_start_block != NULL)
            {   
                curr_block = chain_start_block;
                chain_start_block = chain_start_block->next_header_elem;
                curr_block->next_header_elem = NULL;
            }
                
            elog(ERROR, "Heap buffer overflow");
            return -1;
        }
        header_of_free_block->is_free = false;
        block_addresses[i] = header_of_free_block->block_addr;

        // get new free block
        header_of_free_block = get_free_block_addr();
        prev_header_block->next_header_elem = header_of_free_block;
        prev_header_block = header_of_free_block;
    }

    // operations with last allocated free block
    header_of_free_block->is_free = false;
    block_addresses[count_blocks - 1] = header_of_free_block->block_addr;
    
    return 0;
}

/**
 * \brief Save something to chain that starts with block address start_addr. It can be subchain too.
 * \details This function !COPY! bytes from data to blocks of chain. This means that the function works
 * in time linear to the data size. But its use may be more convenient than copying data directly
 * (of course you can do it too)
 * \param [in] start_addr - address of first chain (or subchain) block
 * \param [in] length - length of saved buffer
 * \param [in] data - buffer to save
 * \return -1 if something went wrong, 0 if all is OK
 */
extern int OOS_save_to_chain(const void *start_addr, const size_t length, const char *data)
{
    Header_elem *header_free_block = get_block_header(start_addr);
    if (header_free_block == NULL || header_free_block->is_free)
    {
        return -1;
    }

    const size_t count_blocks_for_save = length / (size_t) MAX_BLOCK_SIZE +
                                    (length % (size_t) MAX_BLOCK_SIZE > 0);
    size_t curr_data_index = 0;
    for (size_t i = 0; i < count_blocks_for_save; i++)
    {
        if (header_free_block == NULL)
        {
            elog(ERROR, "Heap buffer overflow");
            return -1;
        }

        memcpy(header_free_block->block_addr, &data[curr_data_index], MAX_BLOCK_SIZE);
        header_free_block = header_free_block->next_header_elem;
        curr_data_index += MAX_BLOCK_SIZE;
    }

    return 0;
}

/**
 * \brief free one block
 * \param [in] block_addr - address of block we should free
 */
extern void OOS_free_block(const void *block_addr)
{  
    Header_elem *header_free_block = get_block_header(block_addr);
    if (header_free_block == NULL || header_free_block->is_free)
    {
        return;
    }

    header_free_block->is_free = true;
}

/**
 * \brief free all chain
 * \param [in] block_addr - address of chain start block
 */
extern void OOS_free_chain(const void *block_addr)
{
    Header_elem *header_free_block = get_block_header(block_addr);
    if (header_free_block == NULL || header_free_block->is_free)
    {
        return;
    }

    Header_elem *curr_block;
    header_free_block->is_free = true;
    while (header_free_block->next_header_elem != NULL)
    {
        curr_block = header_free_block;
        header_free_block->is_free = true;
        header_free_block = header_free_block->next_header_elem;
        curr_block->next_header_elem = NULL;
        curr_block->is_free = true;
    }
    

    header_free_block->is_free = true;
}

/**
 * \brief Destroy allocator and free all regions that it allocate
 */
extern void destroy_OOS_allocator()
{
    Guc_data max_allocation_size = get_config_parameter("memory_for_cache");

    size_t count_blocks = max_allocation_size.num / MAX_BLOCK_SIZE;
    size_t header_size = count_blocks * sizeof(Header_elem);

    Header_elem *start = (Header_elem *) start_addr;
    for (size_t i = 0; i < count_blocks; i++)
    {
        munmap(start->block_addr, MAX_BLOCK_SIZE);
        ++start;
    }
    
    munmap(allocated_memory_ptr, sizeof(void *) * count_blocks + header_size);

    allocated_memory_ptr = NULL;
    start_addr = NULL;
}

/**
 * \brief Function for debug. Printing all info about all blocks
 */
extern void print_OOS_alloc_mem()
{
    Guc_data max_allocation_size = get_config_parameter("memory_for_cache");

    Header_elem *curr_block = (Header_elem *) allocated_memory_ptr;
    size_t count_blocks = max_allocation_size.num / MAX_BLOCK_SIZE;
    printf("Block index | Block address | Is free | Is file | Next header element\n");
    for (size_t i = 0; i < count_blocks; i++)
    {
        printf("Block %lu | %p | %d | %d | %p\n", i, curr_block->block_addr,
                curr_block->is_free, curr_block->is_file, curr_block->next_header_elem);
        ++curr_block;
    }
}