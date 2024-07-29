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
#include "../include/memory/cache.h"

void test_alloc()
{
    print_alloc_mem();
    int *a = OOS_allocate(4 * sizeof(int));
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    a[3] = 4;
    printf("%d\n", a[3]);
    print_alloc_mem();

    int *b = OOS_allocate(10 * sizeof(int));
    b[9] = 11;
    printf("%d\n", b[9]);
    print_alloc_mem();

    int *c = OOS_allocate(5 * sizeof(int));
    c[4] = 50;
    printf("%d\n", c[4]);
    print_alloc_mem();

    printf("===================================================\n");
    OOS_free(b);
    print_alloc_mem();

    int *d = OOS_allocate(15 * sizeof(int));
    d[4] = 50;
    printf("%d\n", d[4]);
    print_alloc_mem();

    int *e = OOS_allocate(8 * sizeof(int));
    e[1] = 50;
    printf("%d\n", e[1]);
    print_alloc_mem();

    printf("===================================================\n");
    OOS_free(d);
    print_alloc_mem();

    printf("===================================================\n");
    OOS_free(c);
    print_alloc_mem();

    printf("===================================================\n");
    OOS_free(a);
    print_alloc_mem();

    printf("===================================================\n");
    OOS_free(e);
    print_alloc_mem();
}

void test_cache()
{   
    char buffer[13];
    cache_write("aaaaaa", "Hello world\n", 13, 100000000);
    cache_read("aaaaaa", buffer, 13);
    printf("%s", buffer);
}

int main(int argc, char *argv[]) {
    parse_config();
    init_logger();
    init_cache();
    test_cache();
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