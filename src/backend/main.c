/**
 * The main proccess (the Boss)
 */

#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <libconfig.h>
#include <string.h>
#include <assert.h>
#include "../include/utils/stack.h"
#include "../include/shlib_operations/operations.h"
#include "../include/logger/logger.h"
#include "../include/guc/guc.h"
#include "../include/memory/allocator.h"

void test_alloc()
{
    char **addr = (char **) OOS_allocate_block();
    printf("%p\n", addr);
    OOS_allocate_chain((void **) addr, 2, 2);
    *(addr[0]) = 5;
    *(addr[1]) = 4;
    printf("%p %d, %p %d\n", addr[0], *(addr[0]),  addr[1], *(addr[1]));
    char data[64 * 2];
    memset(data, 'b', 64);
    memset(&data[64], 'a', 64);
    printf("%s\n", data);
    OOS_save_to_chain((void *) addr[0], 64 * 2, data);
    printf("%p %s, %p %s\n", addr[0], addr[0],  addr[1], addr[1]);
    printf("Here\n");
    OOS_free_chain((void *) addr[0]);
    OOS_free_block(addr);
    print_OOS_alloc_mem();
}

int main(int argc, char *argv[]) {
    parse_config();
    init_logger();
    init_OOS_allocator();
    //test_alloc();
    elog(LOG, "Logger inited successfully");

    Guc_data base_dir = get_config_parameter("base_dir");

    Stack_ptr lib_stack = create_stack();
    loader(&lib_stack, base_dir.str);

    if (get_stack_size(lib_stack) > 0)
    {
        init_all_exetensions(lib_stack);
    }
    else
    {
        elog(WARN, "No extensions have been downloaded");
    }

    destroy_OOS_allocator();
    close_all_exetensions(lib_stack);
    stop_logger();

    return 0;
}